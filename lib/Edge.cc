#include "Edge.h"

Edge::Edge()
{}

Edge::Edge(NodePtr U, NodePtr V, FlowType Capacity, RankType Rank /* = 0 */)
	: U_(U), V_(V), Rank_(Rank), Capacity_(Capacity), Flow_(0)
{}

Edge::~Edge()
{}

const NodePtr& Edge::get_U() const {
	return U_;
}

const NodePtr& Edge::get_V() const {
	return V_;
}

const FlowType& Edge::get_flow() const {
	return Flow_;
}

const FlowType& Edge::get_capacity() const {
	return Capacity_;
}

const RankType& Edge::get_rank() const {
	return Rank_;
}

void Edge::set_flow(FlowType Flow) {
	Flow_ = Flow;
}

void Edge::set_capacity(FlowType Capacity) {
	Capacity_ = Capacity;
}