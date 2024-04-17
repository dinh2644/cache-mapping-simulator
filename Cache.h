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
    void directMapped(int cacheSize, int cacheLineSize);
    void setAssociative(int cacheSize, int cacheLineSize);
    void fullyAssociative(int cacheSize, int cacheLineSize);
    void setAssociativeNoAllocOnWriteMiss(int cacheSize, int cacheLineSize);
    void setAssociativeNextLinePrefetch(int cacheSize, int cacheLineSize);
    void prefetchOnMiss(int cacheSize, int cacheLineSize);
    void readFile(string fileName);
    void writeFile(string fileName);
};
