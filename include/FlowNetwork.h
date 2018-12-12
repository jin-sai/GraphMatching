#ifndef FLOWNETWORK_H
#define FLOWNETWORK_H

#include <map>
#include <vector>

#include "Edge.h"
#include "TDefs.h"

// Flow network with capacities between edges.  
class FlowNetwork {
public:
	typedef std::vector<Edge> EdgeList;

private:
	NodeType num_nodes_;                            // number of nodes in the network
	EdgeList edges_;                                // the list of edges (both forward and reverse edges)
	std::vector<std::vector<NodeType> > adj_list_;  // the adjacency list of the network graph 
	std::vector<NodeType> dist_;                    // list to store distances for use in traversal across graph
	std::vector<NodeType> path_to_traverse_;        // list to store the next vertex from which path is to be traversed

	// Returns if there is a path from source to sink by performing a breadth first search
	// Also sets the distance from source to all nodes
	bool path_between(NodeType source, NodeType sink);

	// Sends flow through a path from source to sink computing using depth first search
	// Returns the flow pushed through the path
	FlowType send_flow(NodeType source, NodeType sink, FlowType flow = -1);

	// Finds the S nodes and sets their decomposition labels to 'S'
	void find_S_nodes(NodePtr source);

	// Finds the S nodes and sets their decomposition labels to 'S'
	void find_T_nodes(NodePtr sink);
	
	// Returns whether the edge (u, v) has the decompostion labels
	// of decomposition_label_u and decomposition_label_v respectively
	bool is_edge_label(Edge& edge, IdType decomposition_label_u,
					   IdType decomposition_label_v);
	
	// Returns whether the edge (u, v) is a residual edge
	bool is_residual_edge(Edge& edge, Edge& other_edge);
	
public:
	FlowNetwork();
	FlowNetwork(NodeType num_nodes);
	~FlowNetwork();
	
	// Adds a list of edges to the flow network
	void add_edges(const EdgeList& edges);
	
	// Adds an edge to the flow network
	void add_edge(NodePtr U, NodePtr V, FlowType capacity, RankType rank); 
	
	// Deletes all edges which have the decomposition labels from
	// decomposition_label_u to decomposition_label_v, for instance T->S edges
	void delete_edges(IdType decomposition_label_u, IdType decomposition_label_v);

	// Computes the maximum flow from the source to sink in the flow network
	// Uses Dinic's Algorithm
	// Code for MaxFlow is based on Stanford ACM ICPC Notebook 2015-16
	// with appropriate modifications for use in this library
	FlowType compute_max_flow(NodePtr source, NodePtr sink);
	
	// Decomposes the nodes in the flow network into S, T and U nodes
	void decompose_nodes_STU(NodePtr source, NodePtr sink);
	
	// Returns the residual edges of the residual network corresponding to
	// the flow through the network
	void get_residual_edges(EdgeList& residual_edges, EdgeList& reverse_edges);	
};

#endif