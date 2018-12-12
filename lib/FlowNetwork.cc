#include "FlowNetwork.h"

#include <queue>

FlowNetwork::FlowNetwork()
{}

FlowNetwork::FlowNetwork(NodeType num_nodes)
	: num_nodes_(num_nodes), edges_(0), adj_list_(num_nodes),
	  dist_(num_nodes), path_to_traverse_(num_nodes)
{}

FlowNetwork::~FlowNetwork()
{}

void FlowNetwork::add_edges(const EdgeList& edges) {
	for (auto& edge : edges) {
		add_edge(edge.get_U(), edge.get_V(), edge.get_capacity(), edge.get_rank());
	}
}

void FlowNetwork::add_edge(NodePtr U, NodePtr V, FlowType capacity, RankType rank) {
	if (U->get_id() != V->get_id()) {
		// Both (u,v) and (v,u) edges are added, with 0 capacity for reverse edge
		// This is done for ease of pushing flow across paths
		// edges_[index] and Edges[index ^ 1] are edges in opposite directions

		// Add (u,v) edge with given capacity and rank
		edges_.push_back(Edge(U, V, capacity, rank));
		adj_list_[U->get_id()].push_back(edges_.size() - 1);

		// Add (v,u) edge with 0 capacity and rank
		edges_.push_back(Edge(V, U, 0, 0));
		adj_list_[V->get_id()].push_back(edges_.size() - 1);
	}
}

void FlowNetwork::delete_edges(IdType decomposition_label_u,
							   IdType decomposition_label_v) {
	for (NodeType edge_id = 0; edge_id < (int)edges_.size(); ++edge_id) {
		Edge& edge = edges_[edge_id];

		// Check if edge is between nodes of corresponding decomposition labels
		if (is_edge_label(edge, decomposition_label_u, decomposition_label_v)) { 
			// Delete edge by setting capacity and flow to 0
			edge.set_capacity(0);
			edge.set_flow(0);

			// Set the capacity and flow of reverse edge to 0
			Edge& reverse_edge = edges_[edge_id ^ 1];
			reverse_edge.set_capacity(0);
			reverse_edge.set_flow(0);
		}
	}
}

FlowType FlowNetwork::compute_max_flow(NodePtr source, NodePtr sink) {
	FlowType total_flow = 0;

	// Check if path exists between source and sink
	while (path_between(source->get_id(), sink->get_id())) {
		fill(path_to_traverse_.begin(), path_to_traverse_.end(), 0);

		// Keep sending flow through paths from source to sink 
		while(FlowType flow = send_flow(source->get_id(), sink->get_id())) {
			total_flow += flow;
		}
	}
	return total_flow;
}

void FlowNetwork::decompose_nodes_STU(NodePtr source, NodePtr sink) {
	// Find S nodes which are reachable from source in residual network
	find_S_nodes(source);

	// Find T nodes which can reach sink in residual network
	find_T_nodes(sink);
}

void FlowNetwork::get_residual_edges(EdgeList& residual_edges, EdgeList& reverse_edges) {
	// Iterate through all edges
	for (NodeType edge_id = 0; edge_id < (int)edges_.size(); ++edge_id) {
		Edge& edge = edges_[edge_id];
		Edge& reverse_edge = edges_[edge_id ^ 1];

		// Check if edge is a residual edge
		if (is_residual_edge(edge, reverse_edge)) {
			residual_edges.push_back(edge);
			reverse_edges.push_back(reverse_edge);
		}
	}
}

bool FlowNetwork::path_between(NodeType source, NodeType sink) {
	// Perform a bfs from the source
	std::queue<NodeType> bfs_queue;
	bfs_queue.push(source);

	// Initialise all distances from source to num_nodes + 1
	fill(dist_.begin(), dist_.end(), num_nodes_ + 1);

	// Set distance of source as 0
	dist_[source] = 0;

	// Iterate while there are more nodes reachable from source
	while (!bfs_queue.empty()) {
		NodeType node = bfs_queue.front();
		bfs_queue.pop();

		// On reaching sink, we have a path from source to sink
		if (node == sink) {
			break;
		}

		// Iterate through neighbours of the node
		for (NodeType neigh_id = 0; neigh_id < (int)adj_list_[node].size(); neigh_id++) {
			NodeType neigh_node = adj_list_[node][neigh_id];
			Edge &edge = edges_[neigh_node];

			// Check if more flow can be sent through the edge
			if ((edge.get_flow() < edge.get_capacity()) && 
				(dist_[edge.get_V()->get_id()] > dist_[edge.get_U()->get_id()] + 1)) {
				dist_[edge.get_V()->get_id()] = dist_[edge.get_U()->get_id()] + 1;
				bfs_queue.push(edge.get_V()->get_id());
			}
		}
	}

	// Return if the sink has been reached from the source
	return (dist_[sink] != num_nodes_ + 1);
}


FlowType FlowNetwork::send_flow(NodeType source, NodeType sink, FlowType flow /* = -1 */) {
	// Sending flow is complete when the current node is destination node
	// or if there is no flow to be sent through the path
	if (source == sink || flow == 0) {
		return flow;
	}

	// Iterate through the neighbours which have not yet been traversed
	for (NodeType &path_id = path_to_traverse_[source];
		 path_id < (int)adj_list_[source].size(); ++path_id) {
		Edge &edge = edges_[adj_list_[source][path_id]];

		// Check if the edge (u,v) is an edge in the bfs tree
		if (dist_[edge.get_V()->get_id()] == dist_[edge.get_U()->get_id()] + 1) {
			// Compute limiting flow as minimum flow across all edges in the path
			FlowType limiting_flow = edge.get_capacity() - edge.get_flow();
			if (flow != -1 && limiting_flow > flow) {
				limiting_flow = flow;
			}

			// Recursively send the limiting flow from v to the destination
			if (FlowType pushed_flow = send_flow(edge.get_V()->get_id(), sink, limiting_flow)) {
				// Increment the flow of the edge (u,v) by the pushed flow
				edge.set_flow(edge.get_flow() + pushed_flow);

				// Decrement the flow of the reverse  edge by the pushed flow
				Edge &reverse_edge = edges_[adj_list_[source][path_id] ^ 1];
				reverse_edge.set_flow(reverse_edge.get_flow() - pushed_flow);
				
				return pushed_flow;
			}
		}
	}
	return 0;
}

void FlowNetwork::find_S_nodes(NodePtr source) {
	// Perform a bfs from the source on the residual network
	std::queue<NodePtr> bfs_queue;
	bfs_queue.push(source);
	
	// Initialise all nodes as unvisited (0)
	fill(dist_.begin(), dist_.end(), 0);
	
	// Mark source as visited
	dist_[source->get_id()] = 1;

	// Iterate while there are more nodes reachable from
	// source in the residual network
	while (!bfs_queue.empty()) {
		NodePtr node = bfs_queue.front();
		bfs_queue.pop();
		
		// Set the decomposition label as S
		node->set_decomposition_label("S");
		
		// Iterate through neighbours of the node
		for (NodeType neigh_id = 0; neigh_id < (int)adj_list_[node->get_id()].size(); neigh_id++) {
			NodeType neigh_node = adj_list_[node->get_id()][neigh_id];
			Edge &edge = edges_[neigh_node];
			Edge &reverse_edge = edges_[neigh_node ^ 1];
			
			// Check if edge (u,v) is a residual edge
			if (is_residual_edge(edge, reverse_edge)) {
				// Check that node v is unvisited
				if (dist_[edge.get_V()->get_id()] == 0) {
					// Mark node v as visited
					dist_[edge.get_V()->get_id()] = 1;
					bfs_queue.push(edge.get_V());
				}
			}
		}
	}
}

void FlowNetwork::find_T_nodes(NodePtr sink) {
	// Perform a bfs from the sink on the reverse residual network
	std::queue<NodePtr> bfs_queue;
	bfs_queue.push(sink);
	
	// Initialise all nodes as unvisited (0)
	fill(dist_.begin(), dist_.end(), 0);
	
	// Mark sink as visited
	dist_[sink->get_id()] = 1;
	
	// Iterate while there are more nodes reachable from
	// sink in the reverse residual network
	while (!bfs_queue.empty()) {
		NodePtr node = bfs_queue.front();
		bfs_queue.pop();

		// Set the decomposition label as T
		node->set_decomposition_label("T");

		// Iterate through neighbours of the node
		for (NodeType neigh_id = 0; neigh_id < (int)adj_list_[node->get_id()].size(); neigh_id++) {
			NodeType neigh_node = adj_list_[node->get_id()][neigh_id];
			Edge& edge = edges_[neigh_node];
			Edge &reverse_edge = edges_[neigh_node ^ 1];
			
			// Check if the reverse edge (v,u) is a residual edge
			if (is_residual_edge(reverse_edge, edge)) {
				// Check that v is unvisited
				if (dist_[edge.get_V()->get_id()] != 1) {
					// Mark node v as visited
					dist_[edge.get_V()->get_id()] = 1;
					bfs_queue.push(edge.get_V());
				}
			}
		}
	}
}

bool FlowNetwork::is_edge_label(Edge& edge, IdType decomposition_label_u,
							    IdType decomposition_label_v) {
	// Return if u has label decomposition_label_u and 
	// v has label decomposition_label_v
	return (edge.get_U()->get_decomposition_label() == decomposition_label_u &&
	        edge.get_V()->get_decomposition_label() == decomposition_label_v);
}

bool FlowNetwork::is_residual_edge(Edge& edge, Edge& reverse_edge) {
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