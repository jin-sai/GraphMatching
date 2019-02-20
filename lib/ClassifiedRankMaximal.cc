#include "ClassifiedRankMaximal.h"
#include "Vertex.h"
#include "PartnerList.h"

ClassifiedRankMaximal::ClassifiedRankMaximal(const std::unique_ptr<BipartiteGraph>& G,
                    		        		 bool A_proposing)
    : MatchingAlgorithm(G)
{}

ClassifiedRankMaximal::~ClassifiedRankMaximal()
{}

bool ClassifiedRankMaximal::compute_matching() {
	construct_initial_flow_network();
	
	// Setup the flow network H_0
	H_ = FlowNetwork(nodes_.size());
	H_.add_edges(edges_);

	// Initialise the lists E_[i] of rank i
	initialise_ranked_edge_lists();

	for (auto k = 1; k <= maximum_rank_; ++k) {
		// Add the edges of rank k which have not been deleted
		H_.add_edges(ranked_edges(k));

		// Compute max flow fk
		H_.compute_max_flow(nodes_["source"], nodes_["sink"]);
	
		// Decompose to get Sk, Tk, Uk
		reset_node_decompositions();
		H_.decompose_nodes_STU(nodes_["source"], nodes_["sink"]);

		// Delete edge of form ({T} U {U}) -> {S}
		H_.delete_edges("T", "S");
		H_.delete_edges("U", "S");

		// Delete edges from the ranked edge lists if the applicant
		// leaf is in T or U, or the post leaf is in S or U
		delete_from_ranked_edge_list(k);
	}

	populate_matched_pairs();
	return true;
}

void ClassifiedRankMaximal::construct_initial_flow_network() {
	// Setup the source and the sink
	NodePtr source(new Node("source", 0));
	nodes_["source"] = source;
	NodePtr sink(new Node("sink", 1));
	nodes_["sink"] = sink;
	
	NodeType node_id = 2;
	add_applicant_classification_trees(node_id);
	add_post_classification_trees(node_id);
	add_last_resort_post_classification_trees(node_id);
}

void ClassifiedRankMaximal::add_applicant_classification_trees(NodeType& node_id) {
	const std::unique_ptr<BipartiteGraph>& G = get_graph();
	auto A_partition = G->get_A_partition();

	// Construct the applicant root nodes (C_*_{applicant})
	// Construct edges from source to C_*_{applicant} 
	for (auto& A : A_partition) {
		IdType node_name = get_classification_node_name("*", A.second->get_id());
		NodePtr star_node(new Node(node_name, node_id++));
		nodes_[node_name] = star_node;
		edges_.push_back(Edge(nodes_["source"], star_node, A.second->get_upper_quota()));
	}

	for (auto& A : A_partition) {
		PreferenceList& pref_list = A.second->get_preference_list();
		for (PreferenceList::Iterator it = pref_list.all_begin();
			 it != pref_list.all_end(); ++it) {
			// Construct applicant leaf C_{post}_{applicant}
			// Construct edge from root C_*_{applicant} to applicant leaf C_{post}_{applicant}
			IdType applicant_leaf = get_classification_node_name(it->second->get_id(), A.second->get_id());
			NodePtr leaf_node_applicant(new Node(applicant_leaf, node_id++));
			nodes_[applicant_leaf] = leaf_node_applicant;
			edges_.push_back(Edge(nodes_[get_classification_node_name("*", A.second->get_id())], leaf_node_applicant, 1));
		}
	}
}

void ClassifiedRankMaximal::add_post_classification_trees(NodeType& node_id) {
	const std::unique_ptr<BipartiteGraph>& G = get_graph();
	auto A_partition = G->get_A_partition();
	auto B_partition = G->get_B_partition();
	
	// Construct the post root nodes (C_*_{post})
	// Construct edges from C_*_{post} to sink
	for (auto& B : B_partition) {
		IdType node_name = get_classification_node_name("*", B.second->get_id());
		NodePtr star_node(new Node(node_name, node_id++));
		nodes_[node_name] = star_node;
		edges_.push_back(Edge(star_node, nodes_["sink"], B.second->get_upper_quota()));
	}

	for (auto& A : A_partition) {
		PreferenceList& pref_list = A.second->get_preference_list();
		for (PreferenceList::Iterator it = pref_list.all_begin();
			 it != pref_list.all_end(); ++it) {
			// Construct post leaf C_{applicant}_{post}
			// Construct edge from post leaf C_{applicant}_{post} to root C_*_{post} 
			IdType post_leaf = get_classification_node_name(A.second->get_id(), it->second->get_id());
			NodePtr leaf_node_post(new Node(post_leaf, node_id++));
			nodes_[post_leaf] = leaf_node_post;
			edges_.push_back(Edge(leaf_node_post, nodes_[get_classification_node_name("*", it->second->get_id())], 1));
		}
	}
}

void ClassifiedRankMaximal::add_last_resort_post_classification_trees(NodeType& node_id) {
	const std::unique_ptr<BipartiteGraph>& G = get_graph();
	auto A_partition = G->get_A_partition();
	
	// Construct the last resort post root nodes (C_*_{last_resort_post})
	// Construct edges from C_*_{last_resort_post} to sink
	for (auto& A : A_partition) {
		IdType node_name = get_classification_node_name("*", "L" + A.second->get_id());
		NodePtr star_node(new Node(node_name, node_id++));
		nodes_[node_name] = star_node;
		edges_.push_back(Edge(star_node, nodes_["sink"], 1));
	}

	for (auto& A : A_partition) {
		// Construct the applicant leaf (C_{last_resort_post}_{applicant})
		// Construct edges from root C_*_{last resort post} to applicant leaf C_{last_resort_post}_{applicant}
		IdType applicant_leaf = get_classification_node_name("L" + A.second->get_id(), A.second->get_id());
		NodePtr leaf_node_applicant(new Node(applicant_leaf, node_id++));
		nodes_[applicant_leaf] = leaf_node_applicant;
		edges_.push_back(Edge(nodes_[get_classification_node_name("*", A.second->get_id())], leaf_node_applicant, 1));
		
		// Construct the last resort post leaf (C_{applicant}_{last_resort_post})
		// Construct edges from last resort post leaf C_{applicant}_{last_resort_post} to root C_*_{last resort post}
		IdType post_leaf = get_classification_node_name(A.second->get_id(), "L" + A.second->get_id());
		NodePtr leaf_node_post(new Node(post_leaf, node_id++));
		nodes_[post_leaf] = leaf_node_post;
		edges_.push_back(Edge(leaf_node_post, nodes_[get_classification_node_name("*", "L" + A.second->get_id())], 1));
	}
}

void ClassifiedRankMaximal::initialise_ranked_edge_lists() {
	const std::unique_ptr<BipartiteGraph>& G = get_graph();
	auto A_partition = G->get_A_partition();

	maximum_rank_ = 0;
	for (auto& A : A_partition) {
		PreferenceList& pref_list = A.second->get_preference_list();
		for (PreferenceList::Iterator it = pref_list.all_begin();
			 it != pref_list.all_end(); ++it) {
			NodeType current_rank = it->first;
			if (current_rank > maximum_rank_) {
				maximum_rank_ = current_rank;
			}

			auto edge = make_pair(A.second->get_id(), it->second->get_id());
			E_[current_rank].push_back(make_pair(edge, true));
		}
	}
}

void ClassifiedRankMaximal::construct_matching_from_network() {
	const std::unique_ptr<BipartiteGraph>& G = get_graph();
	FlowNetwork::EdgeList residual_edges, reverse_edges;
	H_.get_residual_edges(residual_edges, reverse_edges);

	// Iterate through all edges in the residual network
	for (NodeType edge_id = 0; edge_id < (int)residual_edges.size(); ++edge_id) {
		auto& edge = residual_edges[edge_id];
		auto& other_edge = reverse_edges[edge_id];

		// Check if edge from post leaf to applicant leaf
		if (is_post_leaf(edge.get_U()->get_name()) &&
		    is_applicant_leaf(edge.get_V()->get_name())) {
			M_flow_[edge.get_V()->get_name()] = make_pair(other_edge.get_rank(),
													    edge.get_U()->get_name());
		}
	}	
	auto A_partition = G->get_A_partition();
}

void ClassifiedRankMaximal::populate_matched_pairs() {
	const std::unique_ptr<BipartiteGraph>& G = get_graph();
	auto A_partition = G->get_A_partition();
	auto B_partition = G->get_B_partition();

	construct_matching_from_network();
	for (auto& matched_pair : M_flow_) {
		IdType applicant = get_vertex_id(matched_pair.first);
		IdType post = get_vertex_id(matched_pair.second.second);

		// Ignore matched pairs containing last resort post
		if (is_last_resort_post(post)) {
			continue;
		}

		// Populate M_ using vertex pointers
		VertexPtr applicant_vertex = A_partition[applicant];
		VertexPtr post_vertex = B_partition[post];
		M_[applicant_vertex].add_partner(make_pair(matched_pair.second.first, post_vertex));
		M_[post_vertex].add_partner(make_pair(1, applicant_vertex));
	}
}

void ClassifiedRankMaximal::reset_node_decompositions() {
	for (auto& node : nodes_) {
		node.second->set_decomposition_label("U");
	}
}

FlowNetwork::EdgeList ClassifiedRankMaximal::ranked_edges(RankType rank) {
	FlowNetwork::EdgeList edges;
	RankedEdgeList& ranked_edge_list = E_[rank];
	for (auto& edge : ranked_edge_list) {
		if (edge.second == false) {
			// Edge has been deleted.
			continue;
		}
		// Obtain applicant and post leaves and add the edge
		IdType applicant_leaf = get_classification_node_name(edge.first.second,
															 edge.first.first);
		IdType post_leaf = get_classification_node_name(edge.first.first,
													    edge.first.second);
		edges.push_back(Edge(nodes_[applicant_leaf], nodes_[post_leaf], 1, rank));
	}
	return edges;
}

void ClassifiedRankMaximal::delete_from_ranked_edge_list(RankType rank) {
	for (RankType j = rank + 1; j <= maximum_rank_; ++j) {
		RankedEdgeList& ranked_edge_list = E_[j];
		for (auto& edge : ranked_edge_list) {
			// Obtain applicant and post leaves
			IdType applicant_leaf = get_classification_node_name(edge.first.second,
																 edge.first.first);
			IdType post_leaf = get_classification_node_name(edge.first.first,
														    edge.first.second);
			if (nodes_[applicant_leaf]->get_decomposition_label() == "T" ||
				nodes_[applicant_leaf]->get_decomposition_label() == "U" ||
				nodes_[post_leaf]->get_decomposition_label() == "S" ||
				nodes_[post_leaf]->get_decomposition_label() == "U") {
				// Delete the edge
				edge.second = false;
			}
		}
	}
}

bool ClassifiedRankMaximal::is_applicant_leaf(IdType nodename) {
	// Extract id1 and id2 from C_{id1}_{id2}
	IdType id1 = get_classification_id(nodename);
	IdType id2 = get_vertex_id(nodename);
	
	// Return true if it is of the form C_{post}_{applicant}
	return (is_applicant(id2) && is_post(id1));
}

bool ClassifiedRankMaximal::is_post_leaf(IdType nodename) {
	// Extract id1 and id2 from C_{id1}_{id2}
	IdType id1 = get_classification_id(nodename);
	IdType id2 = get_vertex_id(nodename);
	
	// Return true if it is of the form C_{applicant}_{post}
	return (is_post(id2) && is_applicant(id1));
}

bool ClassifiedRankMaximal::is_applicant(IdType id) {
	const std::unique_ptr<BipartiteGraph>& G = get_graph();
	auto A_partition = G->get_A_partition();
	// Return true if contained in the A partition
	return (A_partition.find(id) != A_partition.end());
}

bool ClassifiedRankMaximal::is_post(IdType id) {
	const std::unique_ptr<BipartiteGraph>& G = get_graph();
	auto B_partition = G->get_B_partition();
	// Return true if contained in the B parition or
	// if it is a last resort post 
	return (B_partition.find(id) != B_partition.end() ||
	        is_last_resort_post(id));
}

bool ClassifiedRankMaximal::is_last_resort_post(IdType post) {
	// Return true if of the form L{applicant}
	return (post[0] == 'L' && is_applicant(post.substr(1)));
}

IdType ClassifiedRankMaximal::get_classification_node_name(IdType id1, IdType id2) {
	return "C_" + id1 + "_" + id2;
}

IdType ClassifiedRankMaximal::get_vertex_id(IdType nodename) {
	return (nodename.substr(nodename.find_last_of("_") + 1));
}

IdType ClassifiedRankMaximal::get_classification_id(IdType nodename) {
	return (nodename.substr(2, -2 + nodename.find_first_of("_", 2)));	
}