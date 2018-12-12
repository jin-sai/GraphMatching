#ifndef T_DEFS_H
#define T_DEFS_H

#include <map>
#include <memory>

// forward declaration
class Vertex;
class PartnerList;
class Node;

/// Id type for a vertex
typedef std::string IdType;

/// Id type for a node in flow network
typedef int NodeType;

/// Flow type (Integer flow) in flow network
typedef int FlowType;

// Rank representation for a vertex in the preference/partner list
// this must be unique for all the vertex in a list
typedef int RankType;

// Pointer type for vertices
typedef std::shared_ptr<Vertex> VertexPtr;

// Matched partners for every vertex
typedef std::map<std::shared_ptr<Vertex>, PartnerList> MatchedPairListType;

// Pointer type for nodes
typedef std::shared_ptr<Node> NodePtr;

#endif
