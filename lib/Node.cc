#include "Node.h"

Node::Node()
{}

Node::Node(IdType node_name, NodeType node_id) 
	: node_name_(node_name), node_id_(node_id),
	  decomposition_label_("U")
{}

Node::~Node()
{}

const IdType& Node::get_name() const {
	return node_name_;
}

const NodeType& Node::get_id() const {
	return node_id_;
}

const IdType& Node::get_decomposition_label() const {
	return decomposition_label_;
}

void Node::set_decomposition_label(IdType decomposition_label) {
	decomposition_label_ = decomposition_label;
}

