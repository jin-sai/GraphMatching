#include "Edge.h"

Edge::Edge()
{}

Edge::Edge(NodePtr U, NodePtr V, FlowType capacity, RankType rank /* = 0 */)
	: U_(U), V_(V), rank_(rank), capacity_(capacity), flow_(0)
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
	return flow_;
}

const FlowType& Edge::get_capacity() const {
	return capacity_;
}

const RankType& Edge::get_rank() const {
	return rank_;
}

void Edge::set_flow(FlowType flow) {
	flow_ = flow;
}

void Edge::set_capacity(FlowType capacity) {
	capacity_ = capacity;
}