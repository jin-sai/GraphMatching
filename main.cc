#include "GraphReader.h"
#include "BipartiteGraph.h"
#include "PartnerList.h"
#include "MatchingAlgorithm.h"
#include "StableMarriage.h"
#include "RelaxedStable.h"
#include "Popular.h"
#include "RHeuristicHRLQ.h"
#include "HHeuristicHRLQ.h"
#include "YokoiEnvyfreeHRLQ.h"
#include "MaximalEnvyfreeHRLQ.h"
#include "ClassifiedPopular.h"
#include "ClassifiedRankMaximal.h"
#include "Utils.h"
#include <stdexcept>
#include <iostream>
#include <unistd.h>

template<typename T>
void compute_matching(bool A_proposing, const char* input_file, const char* output_file) {
    GraphReader reader(input_file);
    std::unique_ptr<BipartiteGraph> G = reader.read_graph();

    T alg(G, A_proposing);
    if (alg.compute_matching()) {
        auto& M = alg.get_matched_pairs();
        std::ofstream out(output_file);
        print_matching(G, M, out);
    }
    else {
        std::cout << "No popular matching\n";
    }
}

int main(int argc, char* argv[]) {
    int c = 0;
    bool compute_rsm = false;
    bool compute_stable = false;
    bool compute_popular = false;
    bool compute_max_card = false;
    bool compute_rhrlq = false;
    bool compute_hhrlq = false;
    bool compute_yhrlq = false;
    bool compute_ehrlq = false;
    bool compute_cpm = false;
    bool compute_crmm = false;
    bool A_proposing = true;
    const char* input_file = nullptr;
    const char* output_file = nullptr;

    opterr = 0;
    // choose the proposing partition using -A and -B
    // -s, -p, and -m flags compute the stable, max-card popular and pop among
    // max-card matchings respectively
    // -r and -h compute the resident and hopsital heuristic for an HRLQ instance
    // -c computes the many-to-one popular matching
    // -i is the path to the input graph, -o is the path where the matching
    // computed should be stored
    while ((c = getopt(argc, argv, "ABkspmrhyecli:o:")) != -1) {
        switch (c) {
        case 'A': A_proposing = true; break;
        case 'B': A_proposing = false; break;
        case 'k': compute_rsm = true; break;
        case 's': compute_stable = true; break;
        case 'p': compute_popular = true; break;
        case 'm': compute_max_card = true; break;
        case 'r': compute_rhrlq = true; break;
        case 'h': compute_hhrlq = true; break;
        case 'y': compute_yhrlq = true; break;
        case 'e': compute_ehrlq = true; break;
        case 'c': compute_cpm = true; break;
        case 'l': compute_crmm = true; break;
        case 'i': input_file = optarg; break;
        case 'o': output_file = optarg; break;
        case '?':
            if (optopt == 'i') {
                std::cerr << "Option -i requires an argument.\n";
            }
            else if (optopt == 'o') {
                std::cerr << "Option -o requires an argument.\n";
            }
            else {
                std::cerr << "Unknown option: " << (char)optopt << '\n';
            }
            break;
        default: break;
        }
    }

    if (not input_file or not output_file) {
        // do not proceed if file names are not valid
    }
    else if (compute_stable) {
        compute_matching<StableMarriage>(A_proposing, input_file, output_file);
    }
    else if (compute_rsm) {
        compute_matching<RelaxedStable>(A_proposing, input_file, output_file);
    }
    else if (compute_popular) {
        compute_matching<MaxCardPopular>(A_proposing, input_file, output_file);
    }
    else if (compute_max_card) {
        compute_matching<PopularAmongMaxCard>(A_proposing, input_file, output_file);
    }
    else if (compute_rhrlq) {
        compute_matching<RHeuristicHRLQ>(A_proposing, input_file, output_file);
    }
    else if (compute_hhrlq) {
        compute_matching<HHeuristicHRLQ>(A_proposing, input_file, output_file);
    }
    else if (compute_yhrlq) {
        compute_matching<YokoiEnvyfreeHRLQ>(A_proposing, input_file, output_file);
    }
    else if (compute_ehrlq) {
        compute_matching<MaximalEnvyfreeHRLQ>(A_proposing, input_file, output_file);
    }
    else if (compute_cpm) {
        compute_matching<ClassifiedPopular>(A_proposing, input_file, output_file);
    }
    else if (compute_crmm) {
        compute_matching<ClassifiedRankMaximal>(A_proposing, input_file, output_file);
    }

    return 0;
}
