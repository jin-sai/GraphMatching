#ifndef CLASSIFIEDRANKMAXIMAL_H
#define CLASSIFIEDRANKMAXIMAL_H

#include <utility>

#include "MatchingAlgorithm.h"
#include "FlowNetwork.h"

class ClassifiedRankMaximal : public MatchingAlgorithm {
public: 
	typedef std::vector<std::pair<std::pair<IdType, IdType>, bool> > RankedEdgeList;
private:
    std::map<IdType, NodePtr> nodes_;                        // nodes in all classification trees + source + sink
	FlowNetwork::EdgeList edges_;                            // edges in all classification trees + edges from source + edges to sink
	FlowNetwork H_;                                          // flow network constructed using classification trees
    std::map<IdType, std::pair<RankType, IdType> > M_flow_;  // matching based on reverse edges in flow network
    RankType maximum_rank_;                                  // maximum rank among edges in G
    std::map<RankType, RankedEdgeList> E_;                   // lists of edges at every rank 

public:
    ClassifiedRankMaximal(const std::unique_ptr<BipartiteGraph>& G,
                          bool A_proposing);
    virtual ~ClassifiedRankMaximal();

    // Returns true if popular matching exists
    // Computes the popular matching and stores in M_
    bool compute_matching();

private:
    // Populates edges_ and nodes_ for the initial flow network H_0
    // Constructs all applicant and post trees from the given instance
	void construct_initial_flow_network();

	// Adds the applicant classification tree nodes and edges in 
	// the flow network H_0 to nodes_ and edges_
	void add_applicant_classification_trees(NodeType& node_id);
	
	// Adds the post classification tree nodes and edges in 
	// the flow network H_0 to nodes_ and edges_
	void add_post_classification_trees(NodeType& node_id);
	
	// Adds the last resort post classification tree nodes and edges in 
	// the flow network H_0 to nodes_ and edges_
	void add_last_resort_post_classification_trees(NodeType& node_id);

	// Initialises the ranked edge lists and the maximum rank among all edges
	// E_[i] contains edges of rank i
	void initialise_ranked_edge_lists();

	// Constructs the matching (M_flow_) based on the reverse edges
	// from post tree's leaf to applicant tree's leaf in residual network
	void construct_matching_from_network();

	// Populates the matching between applicants and posts into
	// MatchingPairListType MatchingAlgorithm::M_
	void populate_matched_pairs();
	
	// Resets the decomposition label of every node in the graph to U
	void reset_node_decompositions();

	// Returns the list of edges of given rank which have not
	// been deleted already
	FlowNetwork::EdgeList ranked_edges(RankType rank);

	// Deletes from the list of edges higher than the given rank if 
	// the applicant leaf has the decomposition label as U or T, or if
	// the post leaf has the decomposition label as U or S
	void delete_from_ranked_edge_list(RankType rank);

	// Returns true if the node is an applicant leaf
	// Applicant leaf is of form C_{post_node}_{applicant_node}
	bool is_applicant_leaf(IdType nodename);
	
	// Returns true if the node is a post leaf
	// Post leaf is of form C_{applicant_node}_{post_node}
	bool is_post_leaf(IdType nodename);
	
	// Returns true if the id is an applicant
	bool is_applicant(IdType id);

	// Return true if the id is a post
	bool is_post(IdType id);

	// Returns true if the id is a last resort post
	bool is_last_resort_post(IdType id);
	
	// Returns the classification node name from the given ids
	// Classification node name of the form C^{id1}_{id2} is
	// represented here as C_{id1}_{id2}
	IdType get_classification_node_name(IdType id1, IdType id2);

	// Returns the superscript of nodename (id1 in C_{id1}_{id2})
	IdType get_classification_id(IdType nodename);

	// Returns the subscript of nodename (id2 in C_{id1}_{id2})
	IdType get_vertex_id(IdType nodename);
};

#endif