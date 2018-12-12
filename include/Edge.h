#ifndef EDGE_H
#define EDGE_H

#include "Node.h"
#include "TDefs.h"

// Edge between Nodes in a flow network.
class Edge{
private:
	NodePtr U_, V_;      // nodes which contain the edge (U_->V_)
	RankType Rank_;      // rank (if any) of the edge in the original graph
	FlowType Capacity_;  // capacity of the edge
	FlowType Flow_;      // flow through the edge
public:
	Edge();
	Edge(NodePtr U, NodePtr V, FlowType Capacity, RankType Rank = 0);
	~Edge();
	const NodePtr& get_U() const;
	const NodePtr& get_V() const;
	const FlowType& get_flow() const;
	const FlowType& get_capacity() const;
	const RankType& get_rank() const;
	void set_flow(FlowType Flow);
	void set_capacity(FlowType Capacity);
};

#endif