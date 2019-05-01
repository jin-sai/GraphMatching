#ifndef RELAXED_STABLE_H
#define RELAXED_STABLE_H

#include "MatchingAlgorithm.h"

class RelaxedStable : public MatchingAlgorithm {
private:
    bool A_proposing_; // true if vertices from partition A propose, otherwise false

public:
    RelaxedStable(const std::unique_ptr<BipartiteGraph>& G, bool A_proposing = true);
    virtual ~RelaxedStable();
    std::unique_ptr<BipartiteGraph> get_modified_graph();
    bool compute_matching();
    bool is_relaxed_stable(const std::unique_ptr<BipartiteGraph>& G, MatchedPairListType& M);
};

#endif
