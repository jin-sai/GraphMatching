#include "FlowNetwork.h"

#include <queue>

FlowNetwork::FlowNetwork()
{}

FlowNetwork::FlowNetwork(NodeType NumNodes)
	: NumNodes_(NumNodes), Edges_(0), AdjList_(NumNodes),
	  Dist_(NumNodes), PathToTraverse_(NumNodes)
{}

FlowNetwork::~FlowNetwork()
{}

void FlowNetwork::AddEdges(const EdgeList& edges) {
	for (auto& edge : edges) {
		AddEdge(edge.get_U(), edge.get_V(), edge.get_capacity(), edge.get_rank());
	}
}

void FlowNetwork::AddEdge(NodePtr U, NodePtr V, FlowType Capacity, RankType Rank) {
	if (U->get_id() != V->get_id()) {
		// Both (u,v) and (v,u) edges are added, with 0 capacity for reverse edge
		// This is done for ease of pushing flow across paths
		// Edges_[index] and Edges[index ^ 1] are edges in opposite directions

		// Add (u,v) edge with given capacity and rank
		Edges_.push_back(Edge(U, V, Capacity, Rank));
		AdjList_[U->get_id()].push_back(Edges_.size() - 1);

		// Add (v,u) edge with 0 capacity and rank
		Edges_.push_back(Edge(V, U, 0, 0));
		AdjList_[V->get_id()].push_back(Edges_.size() - 1);
	}
}

void FlowNetwork::DeleteEdges(IdType DecompositionU, IdType DecompositionV) {
	for (NodeType edge_id = 0; edge_id < (int)Edges_.size(); ++edge_id) {
		Edge& edge = Edges_[edge_id];

		// Check if edge is between nodes of corresponding decomposition labels
		if (EdgeType(edge, DecompositionU, DecompositionV)) { 
			// Delete edge by setting capacity and flow to 0
			edge.set_capacity(0);
			edge.set_flow(0);

			// Set the capacity and flow of reverse edge to 0
			Edge& reverse_edge = Edges_[edge_id ^ 1];
			reverse_edge.set_capacity(0);
			reverse_edge.set_flow(0);
		}
	}
}

FlowType FlowNetwork::MaxFlow(NodePtr Source, NodePtr Sink) {
	FlowType total_flow = 0;

	// Check if path exists between Source and Sink
	while (PathBetween(Source->get_id(), Sink->get_id())) {
		fill(PathToTraverse_.begin(), PathToTraverse_.end(), 0);

		// Keep sending flow through paths from source to sink 
		while(FlowType flow = SendFlow(Source->get_id(), Sink->get_id())) {
			total_flow += flow;
		}
	}
	return total_flow;
}

void FlowNetwork::DecomposeNodesSTU(NodePtr Source, NodePtr Sink) {
	// Find S nodes which are reachable from Source in residual network
	FindSNodes(Source);

	// Find T nodes which can reach Sink in residual network
	FindTNodes(Sink);
}

void FlowNetwork::GetResidualEdges(EdgeList& residual_edges, EdgeList& reverse_edges) {
	// Iterate through all edges
	for (NodeType edge_id = 0; edge_id < (int)Edges_.size(); ++edge_id) {
		Edge& edge = Edges_[edge_id];
		Edge& reverse_edge = Edges_[edge_id ^ 1];

		// Check if edge is a residual edge
		if (IsResidualEdge(edge, reverse_edge)) {
			residual_edges.push_back(edge);
			reverse_edges.push_back(reverse_edge);
		}
	}
}

bool FlowNetwork::PathBetween(NodeType Source, NodeType Sink) {
	// Perform a bfs from the source
	std::queue<NodeType> bfs_queue;
	bfs_queue.push(Source);

	// Initialise all distances from source to num_nodes + 1
	fill(Dist_.begin(), Dist_.end(), NumNodes_ + 1);

	// Set distance of source as 0
	Dist_[Source] = 0;

	// Iterate while there are more nodes reachable from source
	while (!bfs_queue.empty()) {
		NodeType node = bfs_queue.front();
		bfs_queue.pop();

		// On reaching sink, we have a path from source to sink
		if (node == Sink) {
			break;
		}

		// Iterate through neighbours of the node
		for (NodeType neigh_id = 0; neigh_id < (int)AdjList_[node].size(); neigh_id++) {
			NodeType neigh_node = AdjList_[node][neigh_id];
			Edge &edge = Edges_[neigh_node];

			// Check if more flow can be sent through the edge
			if ((edge.get_flow() < edge.get_capacity()) && 
				(Dist_[edge.get_V()->get_id()] > Dist_[edge.get_U()->get_id()] + 1)) {
				Dist_[edge.get_V()->get_id()] = Dist_[edge.get_U()->get_id()] + 1;
				bfs_queue.push(edge.get_V()->get_id());
			}
		}
	}

	// Return if the sink has been reached from the source
	return (Dist_[Sink] != NumNodes_ + 1);
}


FlowType FlowNetwork::SendFlow(NodeType CurrentNode, NodeType DestNode, FlowType Flow /* = -1 */) {
	// Sending flow is complete when the current node is destination node
	// or if there is no flow to be sent through the path
	if (CurrentNode == DestNode || Flow == 0) {
		return Flow;
	}

	// Iterate through the neighbours which have not yet been traversed
	for (NodeType &path_id = PathToTraverse_[CurrentNode];
		 path_id < (int)AdjList_[CurrentNode].size(); ++path_id) {
		Edge &edge = Edges_[AdjList_[CurrentNode][path_id]];

		// Check if the edge (u,v) is an edge in the bfs tree
		if (Dist_[edge.get_V()->get_id()] == Dist_[edge.get_U()->get_id()] + 1) {
			// Compute limiting flow as minimum flow across all edges in the path
			FlowType limiting_flow = edge.get_capacity() - edge.get_flow();
			if (Flow != -1 && limiting_flow > Flow) {
				limiting_flow = Flow;
			}

			// Recursively send the limiting flow from v to the destination
			if (FlowType pushed_flow = SendFlow(edge.get_V()->get_id(), DestNode, limiting_flow)) {
				// Increment the flow of the edge (u,v) by the pushed flow
				edge.set_flow(edge.get_flow() + pushed_flow);

				// Decrement the flow of the reverse  edge by the pushed flow
				Edge &reverse_edge = Edges_[AdjList_[CurrentNode][path_id]^1];
				reverse_edge.set_flow(reverse_edge.get_flow() - pushed_flow);
				
				return pushed_flow;
			}
		}
	}
	return 0;
}

void FlowNetwork::FindSNodes(NodePtr Source) {
	// Perform a bfs from the source on the residual network
	std::queue<NodePtr> bfs_queue;
	bfs_queue.push(Source);
	
	// Initialise all nodes as unvisited (0)
	fill(Dist_.begin(), Dist_.end(), 0);
	
	// Mark source as visited
	Dist_[Source->get_id()] = 1;

	// Iterate while there are more nodes reachable from
	// source in the residual network
	while (!bfs_queue.empty()) {
		NodePtr node = bfs_queue.front();
		bfs_queue.pop();
		
		// Set the decomposition label as S
		node->set_decomposition_label("S");
		
		// Iterate through neighbours of the node
		for (NodeType neigh_id = 0; neigh_id < (int)AdjList_[node->get_id()].size(); neigh_id++) {
			NodeType neigh_node = AdjList_[node->get_id()][neigh_id];
			Edge &edge = Edges_[neigh_node];
			Edge &reverse_edge = Edges_[neigh_node ^ 1];
			
			// Check if edge (u,v) is a residual edge
			if (IsResidualEdge(edge, reverse_edge)) {
				// Check that node v is unvisited
				if (Dist_[edge.get_V()->get_id()] == 0) {
					// Mark node v as visited
					Dist_[edge.get_V()->get_id()] = 1;
					bfs_queue.push(edge.get_V());
				}
			}
		}
	}
}

void FlowNetwork::FindTNodes(NodePtr Sink) {
	// Perform a bfs from the sink on the reverse residual network
	std::queue<NodePtr> bfs_queue;
	bfs_queue.push(Sink);
	
	// Initialise all nodes as unvisited (0)
	fill(Dist_.begin(), Dist_.end(), 0);
	
	// Mark sink as visited
	Dist_[Sink->get_id()] = 1;
	
	// Iterate while there are more nodes reachable from
	// sink in the reverse residual network
	while (!bfs_queue.empty()) {
		NodePtr node = bfs_queue.front();
		bfs_queue.pop();

		// Set the decomposition label as T
		node->set_decomposition_label("T");

		// Iterate through neighbours of the node
		for (NodeType neigh_id = 0; neigh_id < (int)AdjList_[node->get_id()].size(); neigh_id++) {
			NodeType neigh_node = AdjList_[node->get_id()][neigh_id];
			Edge& edge = Edges_[neigh_node];
			Edge &reverse_edge = Edges_[neigh_node ^ 1];
			
			// Check if the reverse edge (v,u) is a residual edge
			if (IsResidualEdge(reverse_edge, edge)) {
				// Check that v is unvisited
				if (Dist_[edge.get_V()->get_id()] != 1) {
					// Mark node v as visited
					Dist_[edge.get_V()->get_id()] = 1;
					bfs_queue.push(edge.get_V());
				}
			}
		}
	}
}

bool FlowNetwork::EdgeType(Edge& edge, IdType DecompositionU, IdType DecompositionV) {
	// Return if u has label DecompositionU and v has label DecompositionV
	return (edge.get_U()->get_decomposition_label() == DecompositionU &&
	        edge.get_V()->get_decomposition_label() != DecompositionV);
}

bool FlowNetwork::IsResidualEdge(Edge& edge, Edge& reverse_edge) {
	// If edge is a forward edge, residual edge exists if flow < capacity
	if (edge.get_flow() < edge.get_capacity()) {
		return true;
	}

	// If edge is a reverse edge, residual edge exists if reverse flow > 0
	if (reverse_edge.get_flow() > 0) {
		return true;
	}

	return false;
}