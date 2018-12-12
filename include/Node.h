#ifndef NODE_H
#define NODE_H

#include "TDefs.h"

// Node in a flow network.
class Node {
private:
	IdType NodeName_;       // name of the node (for instance, C_a_b)
	NodeType Id_;           // id of this node
	IdType Decomposition_;  // decomposition label for the node (S or T or U)

public:
	Node();
	Node(IdType NodeName, NodeType Id);
	~Node();
	const IdType& get_name() const;
	const NodeType& get_id() const;
	const IdType& get_decomposition_label() const;
	void set_decomposition_label(IdType Decomposition);
};

#endif