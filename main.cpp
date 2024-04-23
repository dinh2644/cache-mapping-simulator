#include <iostream>
#include "Cache.h"
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        cerr << "Must have 3 arguments: ./cache-sim inputFileName outputFileName" << endl;
        return 1;
    }
    string inputFileName = argv[1];
    string outputFileName = argv[2];

    int cacheSize[4] = {1024, 4096, 16384, 32768};
    int cacheArrSize = sizeof(cacheSize) / sizeof(cacheSize[0]);

    int ways[4] = {2, 4, 8, 16};
    int waysArrSize = sizeof(ways) / sizeof(ways[0]);

    Cache c;
    c.readFile(inputFileName);

    // Direct-Mapped Cache
    for (int i = 0; i < cacheArrSize; i++)
    {
        c.directMapped(cacheSize[i], 32);
    }

    // Set-Associative Cache
    for (int i = 0; i < waysArrSize; i++)
    {
        c.setAssociative(16384, 32, ways[i]);
    }

    // Fully-Associative cache (LRU)
    c.fullyAssociative(16384, 32, "LRU");

    // Fully-Associative cache (pLRU)
    c.fullyAssociative(16384, 32, "pLRU");

    // Set-Associative Cache with no Allocation on a Write Miss
    for (int i = 0; i < waysArrSize; i++)
    {
        c.setAssociativeNoAllocOnWriteMiss(16384, 32, ways[i]);
    }

    // Set-Associative Cache with Next-line Prefetching
    for (int i = 0; i < waysArrSize; i++)
    {
        c.setAssociativeNextLinePrefetch(16384, 32, ways[i]);
    }

    // Prefetch-on-a-Miss
    for (int i = 0; i < waysArrSize; i++)
    {
        c.prefetchOnMiss(16384, 32, ways[i]);
    }

    c.writeFile(outputFileName);

    return 0;
}
