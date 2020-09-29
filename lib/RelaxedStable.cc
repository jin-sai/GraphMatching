#include "RelaxedStable.h"
#include "ClassifiedPopular.h"
#include "Vertex.h"
#include "PartnerList.h"
#include "Utils.h"
#include <stack>
#include <map>
#include <iostream>

RelaxedStable::RelaxedStable(const std::unique_ptr<BipartiteGraph>& G,
    bool A_proposing)
    : MatchingAlgorithm(G), A_proposing_(A_proposing)
{}

RelaxedStable::~RelaxedStable()
{}

//check if output matching is relaxed stable
bool RelaxedStable::is_relaxed_stable(const std::unique_ptr<BipartiteGraph>& G, MatchedPairListType& M) {
    /////////////////////////////////
    //print_matching(G, M, std::cout);
    //map to maintain residents which are in blocking pair
    std::map<VertexPtr, bool> in_blocking_pair;
    //for each resident
    auto A_partition = G->get_A_partition();
    for (auto& A1 : A_partition) {
        auto u = A1.second;
        in_blocking_pair[u] = false;

        //resetting u's preference list's proposal index to 0
        PreferenceList& u_pref_list = u->get_preference_list();

        auto u_partnerlist = M[u];
        //iterate through u's preference list
        for (PreferenceList::Iterator it = u_pref_list.all_begin();
            it != u_pref_list.all_end(); ++it) {
            //hospital from u's pref list to check if (u,v) is blocking pair
            auto v = u_pref_list.get_vertex(it - u_pref_list.all_begin());

            //if this hospital is matched to this resident then no need to check for later hospitals
            if (u_partnerlist.find(v) != u_partnerlist.cend()) {
                break;
            }

            ///////////////////////redundant
            //if resident is matched
            //if (u_partnerlist.size() > 0) {
                //the hospital to which u is matched
              //  auto v_alloted = u_partnerlist.get_vertex(u_partnerlist.begin());
                //if (u_pref_list.is_ranked_better(v_alloted, v)) {
                  //  continue;
                //}
            //}
            //////////////////////////////


            PreferenceList& v_pref_list = v->get_preference_list();
            auto v_partnerlist = M_[v];
            //if v is fully subscribed
            if (v_partnerlist.size() >= v->get_upper_quota()) {
                auto u_least_preferred = v_partnerlist.get_vertex(v_partnerlist.get_least_preferred());
                if (v_pref_list.is_ranked_better(u_least_preferred, u)) {
                    ////////////////////////////////////////////////////////////////
                    //std::cout << v->get_id() << " doesnot prefers " << u_least_preferred->get_id() << " over " << u->get_id() << "\n";
                    continue;
                }
                ////////////////////////////////////////////////////////////////
                //std::cout << v->get_id() << " prefers " << u_least_preferred->get_id() << " over "<<u->get_id()<<"\n";
            }
            in_blocking_pair[u] = true;
            in_blocking_pair[v] = true;
            ////////////////////////////////////////////////////////////////
            //std::cout << u->get_id() << " , " << v->get_id() << " blocking pair\n";
        }
        // if u is unmatched
        if (u_partnerlist.size() == 0 && in_blocking_pair[u] == true) {
            ////////////////////////////////////////////////////////////
            //std::cout << u->get_id() << " is unmatched and in blocking pair\n";
            return false;
        }
    }
    //for each hospital
    auto B_partition = G->get_B_partition();
    for (auto& B1 : B_partition) {
        auto v = B1.second;
        auto v_lower_quota = v->get_lower_quota();
        auto v_partnerlist = M_[v];
        for (auto pit = v_partnerlist.cbegin(), pie = v_partnerlist.cend(); pit != pie; ++pit) {
            auto u = v_partnerlist.get_vertex(pit);
            if (in_blocking_pair[u] == true) {
                v_lower_quota = v_lower_quota - 1;
            }
        }
        if (v_lower_quota < 0) {
            /////////////////////////////////////////////////////
            //std::cout << v->get_id() << " has more blocking residents "<<v_lower_quota<<"\n";
            return false;
        }
    }
    return true;
}

//modified graph to input to classified popular
std::unique_ptr<BipartiteGraph> RelaxedStable::get_modified_graph() {
    //old graph and its partitions
    const std::unique_ptr<BipartiteGraph>& G = get_graph();
    auto A_partition = G->get_A_partition();
    auto B_partition = G->get_B_partition();

    //partitions of new graph
    BipartiteGraph::ContainerType A, B;

    //for each resident create a new vertex and place it in new graph
    for (auto& A1 : A_partition) {
        A.emplace(A1.first, std::make_shared<Vertex>(A1.first, A1.second->get_lower_quota(), A1.second->get_upper_quota()));
    }
    //for each hospital create a new vertex with upper_quota as lower_quota and place it in new graph
    for (auto& B1 : B_partition) {
        B.emplace(B1.first, std::make_shared<Vertex>(B1.first, B1.second->get_lower_quota(), B1.second->get_lower_quota()));
    }
    //for each resident in new graph create a new preference list
    for (auto& A1 : A_partition) {
        VertexPtr v = A[A1.first];
        PreferenceList& new_pref_list = v->get_preference_list();
        PreferenceList& pref_list = A1.second->get_preference_list();
        for (PreferenceList::Iterator it = pref_list.all_begin();
            it != pref_list.all_end(); ++it) {
            // add first element in preference list with rank 1
            if (it == pref_list.all_begin()) {
                new_pref_list.emplace_back(B[it->second->get_id()]);
            }
            // add all other elements in preference list with same rank 1
            else {
                new_pref_list.emplace_back_with_tie(B[it->second->get_id()]);
            }
        }
    }
    return std::make_unique<BipartiteGraph>(A, B);
}

bool RelaxedStable::compute_matching() {
    
    const std::unique_ptr<BipartiteGraph>& G = get_graph();
    auto A_partition = G->get_A_partition();
    auto B_partition = G->get_B_partition();

    //For finding minimal feasible matching we need modified graph
    const std::unique_ptr<BipartiteGraph>& G1 = get_modified_graph();
    ClassifiedPopular alg(G1, A_proposing_);
    if (alg.compute_matching()) {
        //output matching of classified popular class
        MatchedPairListType M1_ = alg.get_matched_pairs();
        FlowType required_flow = 0, total_flow = 0;

        //construct the above matching for the original graph with original ranks
        for (auto& it : G1->get_B_partition()) {
            auto u = it.second;
            auto M_u = M1_.find(u);
            required_flow = required_flow + u->get_upper_quota();
            if (M_u != M1_.end()) {
                //getting new vertex data
                auto u_new = B_partition[u->get_id()];
                auto& u_new_pref_list = u_new->get_preference_list();
                auto& u_new_partner_list = M_[u_new];

                auto& partners = M_u->second;
                for (auto pit = partners.cbegin(), pie = partners.cend(); pit != pie; ++pit) {
                    total_flow++;
                    auto v = partners.get_vertex(pit);
                    auto v_new = A_partition[v->get_id()];
                    auto& v_new_pref_list = v_new->get_preference_list();
                    auto& v_new_partner_list = M_[v_new];

                    // u's rank on v's preference list
                    auto u_rank = v_new_pref_list.get_rank(v_new_pref_list.find(u_new));
                    // v's rank on u's preference list
                    auto v_rank = u_new_pref_list.get_rank(u_new_pref_list.find(v_new));

                    // add to new matchings
                    u_new_partner_list.add_partner(std::make_pair(v_rank, v_new));
                    v_new_partner_list.add_partner(std::make_pair(u_rank, u_new));
                }
            }
        }
        //////////////////////////////////
        //std::cout << "after classical popular------------------------\n";
        ///print_matching(G, M_, std::cout);
        //std::cout << "-----------------------------------------------\n";

        // if there is no feasible matching
        //if (total_flow != required_flow) {
          //  std::cout << "No feasible matching\n";
            //return false;
        //}

        //After finding minimal feasible matching
        std::map<VertexPtr, int> level;
        std::stack<VertexPtr> free_list;
        //Add each unmatched resident to free_list
        for (auto& A1 : A_partition) {
            auto v = A1.second;
            level[v] = 0;
            //if resident is unmatched add to freelist and make level 1
            if (M_.find(v) == M_.end()) {
                ///////////////////////////////////////////
                //if (v->get_id() == "r159") {
                  //  std::cout << v->get_id() << " unmatched\n";
                //}

                free_list.push(v);
                level[v] = 1;
            }
        }

        //while there are unmatched residents
        while (!free_list.empty()) {
            // getting top most resident from list and removing it from free list
            auto u = free_list.top();
            auto& u_pref_list = u->get_preference_list();
            auto& u_partner_list = M_[u];
            free_list.pop();

            // if resident did not exhaust its preference list
            if (!u_pref_list.empty()) {
                // highest ranked vertex to whom u not yet proposed
                auto v = u_pref_list.get_vertex(u_pref_list.get_proposal_index());

                ////////////////////////////////
                //if (u->get_id() == "r159") {
                  //  std::cout << u->get_id() << " proposed to " << v->get_id() << "\n";
                //}
                
                // v's preference list and list of partners
                auto& v_pref_list = v->get_preference_list();
                auto& v_partner_list = M_[v];

                // u's rank on v's preference list
                auto u_rank = v_pref_list.get_rank(v_pref_list.find(u));

                // v's rank on u's preference list
                auto v_rank = u_pref_list.get_rank(u_pref_list.get_proposal_index());

                // if v is undersubscribed
                if (v_partner_list.size() < v->get_upper_quota()) {
                    ////////////////////////////////
                    //if (u->get_id() == "r159") {
                      //  std::cout << u->get_id() << " accepted to "<<v->get_id()<<" 1st case\n";
                    //}

                    // accept the proposal
                    u_partner_list.add_partner(std::make_pair(v_rank, v));
                    v_partner_list.add_partner(std::make_pair(u_rank, u));
                }
                // if v is fully subscriibed
                else {
                    auto worst_partner1 = v_partner_list.cend();
                    //check for level 0 residents matched to v
                    for (auto pit = v_partner_list.cbegin(), pie = v_partner_list.cend(); pit != pie; ++pit) {
                        auto possible_partner = v_partner_list.get_vertex(pit);
                        if (level[possible_partner] == 0) {
                            worst_partner1 = pit;
                            break;
                        }
                    }
                    // if level 0 resident found
                    if (worst_partner1 != v_partner_list.cend()) {
                        // level 0 resident and its partner list
                        auto uc = v_partner_list.get_vertex(worst_partner1);
                        auto& uc_partner_list = M_[uc];

                        //set level of level 0 resident to 1
                        level[uc] = 1;

                        // remove (uc,v) from matchings
                        v_partner_list.remove(uc);
                        uc_partner_list.remove(v);

                        // add (u, v) in the matching
                        u_partner_list.add_partner(std::make_pair(v_rank, v));
                        v_partner_list.add_partner(std::make_pair(u_rank, u));

                        ////////////////////////////////
                        //if (u->get_id() == "r159") {
                          //  std::cout << u->get_id() << " replaced level 0 resident " << uc->get_id() << "\n";
                        //}

                        //add uc to free list
                        free_list.push(uc);
                    }
                    // if there is no level 0 resident
                    else {
                        // v's least preferred partner
                        auto worst_partner = v_partner_list.get_least_preferred();

                        // worst partner,  rank, and matched partners
                        auto uc = v_partner_list.get_vertex(worst_partner);
                        auto uc_rank = v_partner_list.get_rank(worst_partner);
                        auto& uc_partner_list = M_[uc];

                        // if v prefers u over its worst partner
                        if (u_rank < uc_rank) {
                            // remove (uc,v) from matchings
                            v_partner_list.remove_least_preferred();
                            uc_partner_list.remove(v);

                            // add (u, v) in the matching
                            u_partner_list.add_partner(std::make_pair(v_rank, v));
                            v_partner_list.add_partner(std::make_pair(u_rank, u));

                            ////////////////////////////////
                            //if (u->get_id() == "r159") {
                            //    std::cout << u->get_id() << " replaced less preferred resident " << uc->get_id() << "\n";
                            //}

                            // push uc to free list
                            free_list.push(uc);
                        }
                        else {
                            free_list.push(u);
                        }
                    }
                }
                u_pref_list.move_proposal_index();
            }
        }
        if (is_relaxed_stable(G, M_)) {
            std::cout << "Relaxed stable\n";
        }
        else {
            std::cout << " Not Relaxed stable\n";
        }
        return true;
    }
    else {
        std::cout << "No feasible matching\n";
        return false;
    }
}
