#include <iostream>
#include "Trace.h"
#include <vector>
using namespace std;

class Cache
{
private:
    vector<Trace> tracesVect;
    vector<unsigned long long> result;

public:
    void directMapped();
    void setAssociative();
    void fullyAssociative();
    void setAssociativeNoAllocOnWriteMiss();
    void setAssociativeNextLinePrefetch();
    void prefetchOnMiss();
    void readFile(string fileName);
    void writeFile(string fileName);
};
