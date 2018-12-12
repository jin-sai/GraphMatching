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
	NodeType NumNodes_;                            // number of nodes in the network
	EdgeList Edges_;                               // the list of edges (both forward and reverse edges)
	std::vector<std::vector<NodeType> > AdjList_;  // the adjacency list of the network graph 
	std::vector<NodeType> Dist_;                   // list to store distances for use in traversal across graph
	std::vector<NodeType> PathToTraverse_;         // list to store the next vertex from which path is to be traversed

	// Returns if there is a path from source to sink by performing a breadth first search
	// Also sets the distance from source to all nodes
	bool PathBetween(NodeType Source, NodeType Sink);

	// Sends flow through a path from source to sink computing using depth first search
	// Returns the flow pushed through the path
	FlowType SendFlow(NodeType Source, NodeType Sink, FlowType flow = -1);

	// Finds the S nodes and sets their decomposition labels to 'S'
	void FindSNodes(NodePtr Source);

	// Finds the S nodes and sets their decomposition labels to 'S'
	void FindTNodes(NodePtr Sink);
	
	// Returns whether the edge (U, V) has the decompostion labels
	// of DecompositionU and DecompositionV respectively
	bool EdgeType(Edge& edge, IdType DecompositionU, IdType DecompositionV);
	
	// Returns whether the edge (U, V) is a residual edge
	bool IsResidualEdge(Edge& edge, Edge& other_edge);
	
public:
	FlowNetwork();
	FlowNetwork(NodeType NumNodes);
	~FlowNetwork();
	
	// Adds a list of edges to the flow network
	void AddEdges(const EdgeList& edges);
	
	// Adds an edge to the flow network
	void AddEdge(NodePtr U, NodePtr V, FlowType Capacity, RankType Rank); 
	
	// Deletes all edges which have the decomposition labels from
	// DecompositionU to DecompositionV, for instance T->S edges
	void DeleteEdges(IdType DecompositionU, IdType DecompositionV);

	// Computes the maximum flow from the source to sink in the flow network
	// Uses Dinic's Algorithm
	// Code for MaxFlow is based on Stanford ACM ICPC Notebook 2015-16
	// with appropriate modifications for use in this library
	FlowType MaxFlow(NodePtr Source, NodePtr Sink);
	
	// Decomposes the nodes in the flow network into S, T and U nodes
	void DecomposeNodesSTU(NodePtr Source, NodePtr Sink);
	
	// Returns the residual edges of the residual network corresponding to
	// the flow through the network
	void GetResidualEdges(EdgeList& residual_edges, EdgeList& reverse_edges);	
};

#endif