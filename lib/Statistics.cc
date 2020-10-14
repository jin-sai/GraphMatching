#include "Statistics.h"
#include "StableMarriage.h"
#include "Vertex.h"
#include "PartnerList.h"
#include "Utils.h"
#include <map>
#include <iostream>

Statistics::Statistics()
{
    //initially set all statistics to zero
    S = 0;
    BPC = 0;
    BR = 0;
    R1 = 0;
    def = 0;
}

Statistics::~Statistics()
{}

//To get the statistics of matching with respect to graph
void Statistics::get_statistics(const std::unique_ptr<BipartiteGraph>& G, MatchedPairListType& M) {
    
    std::map<VertexPtr, bool> in_blocking_pair;
    //for each resident
    auto A_partition = G->get_A_partition();
    for (auto& A1 : A_partition) {
        auto u = A1.second;
        in_blocking_pair[u] = false;
        PreferenceList& u_pref_list = u->get_preference_list();
        auto u_partnerlist = M[u];
        //iterate through u's preference list
        for (PreferenceList::Iterator it = u_pref_list.all_begin();
            it != u_pref_list.all_end(); ++it) {
            //hospital from u's pref list to check if (u,v) is blocking pair
            auto v = u_pref_list.get_vertex(it - u_pref_list.all_begin());
            //if this hospital is matched to this resident then no need to check for later hospitals
            if (u_partnerlist.find(v) != u_partnerlist.cend()) {
                // v's rank on u's preference list
                auto v_rank = u_pref_list.get_rank(it - u_pref_list.all_begin());
                //if v is ranked 1 in u's preference list
                if (v_rank == 1) {
                    R1++;
                }
                break;
            }

            PreferenceList& v_pref_list = v->get_preference_list();
            auto v_partnerlist = M[v];
            //if v is fully subscribed
            if (v_partnerlist.size() == v->get_upper_quota()) {
                auto u_least_preferred = v_partnerlist.get_vertex(v_partnerlist.get_least_preferred());
                //if v doesnt prefer u to its least preferred then not a blocking pair
                if (v_pref_list.is_ranked_better(u_least_preferred, u)) {
                    continue;
                }
            }
            //if v is not fully subscribed or
            //if v prefers u to its least preferred then (u,v) is a blocking pair
            in_blocking_pair[u] = true;
            //increment number of blocking pairs
            BPC++;
        }
        // if u is matched, increment size of matching
        if (u_partnerlist.size() > 0) {
            S++;
        }
        // if this resident is in some blocking pair
        if (in_blocking_pair[u] == true) {
            BR++;
        }
    }

    StableMarriage alg(G, true);
    alg.compute_matching();
    auto& M1 = alg.get_matched_pairs();

    //for each hospital
    auto B_partition = G->get_B_partition();

    unsigned long int sum_of_lower_quota = 0;
    for (auto& B1 : B_partition) {
        auto v = B1.second;
        auto v_lower_quota = v->get_lower_quota();
        sum_of_lower_quota += v_lower_quota;
        auto v_partnerlist = M1[v];
        if (v_partnerlist.size() < v_lower_quota) {
            def += v_lower_quota - v_partnerlist.size();
        }
    }
    std::cout << S << "," << BPC << "," << BR << "," << R1 << "," << def << ","<<sum_of_lower_quota<<"\n";
}
