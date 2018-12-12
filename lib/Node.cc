#include "Node.h"

Node::Node()
{}

Node::Node(IdType NodeName, NodeType Id) 
	: NodeName_(NodeName), Id_(Id), Decomposition_("U")
{}

Node::~Node()
{}

const IdType& Node::get_name() const {
	return NodeName_;
}

const NodeType& Node::get_id() const {
	return Id_;
}

const IdType& Node::get_decomposition_label() const {
	return Decomposition_;
}

void Node::set_decomposition_label(IdType Decomposition) {
	Decomposition_ = Decomposition;
}

