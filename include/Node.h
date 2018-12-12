#ifndef NODE_H
#define NODE_H

#include "TDefs.h"

// Node in a flow network.
class Node {
private:
	IdType node_name_;            // name of the node (for instance, C_a_b)
	NodeType node_id_;            // id of this node
	IdType decomposition_label_;  // decomposition label for the node (S or T or U)

public:
	Node();
	Node(IdType node_name, NodeType node_id);
	~Node();
	const IdType& get_name() const;
	const NodeType& get_id() const;
	const IdType& get_decomposition_label() const;
	void set_decomposition_label(IdType decomposition_label);
};

#endif