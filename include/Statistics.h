#ifndef STATISTICS_H
#define STATISTICS_H

#include "BipartiteGraph.h"

class Statistics {
public:
    //size of matching
    unsigned long int S;
    //number of blocking pairs
    unsigned long int BPC;
    //number of blocking residents
    unsigned long int BR;
    //number of residents matched to their rank 1 hospitals
    unsigned long int R1;
    //Deficiency of graph
    unsigned long int def;

public:
    Statistics();
    virtual ~Statistics();
    void get_statistics(const std::unique_ptr<BipartiteGraph>& G, MatchedPairListType& M);
};

#endif
