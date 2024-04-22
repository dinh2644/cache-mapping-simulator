#include <iostream>
#include "Cache.h"
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

int main(int argc, char const *argv[])
{
    // if (argc != 3)
    // {
    //     cerr << "Must have 3 arguments. ./cache-sim inputFileName outputFileName" << endl;
    //     return 1;
    // }
    // string inputFileName = argv[1];
    // string outputFileName = argv[2];

    // Caches sizez: 1KB, 4KB, 16KB, 32KB
    int cacheSize[4] = {1024, 4096, 16384, 32768};
    int numCacheSize = sizeof(cacheSize) / sizeof(cacheSize[0]);

    // associativity: 2, 4, 8, 16
    int ways[4] = {2, 4, 8, 16};
    int waySize = sizeof(cacheSize) / sizeof(cacheSize[0]);

    Cache c;
    c.readFile("project2/traces/trace1.txt");

    // for (int i = 0; i < numCacheSize; i++)
    // {
    //     // (cacheSize, cacheLineSize)
    //     c.directMapped(cacheSize[i], 32);
    // }

    // for (int i = 0; i < waySize; i++)
    // {
    //     // (cacheSize, cacheLineSize, nWay)
    //     c.setAssociative(16384, 32, ways[i]);
    // }

    c.fullyAssociative(16384, 32, "pLRU");

    // for (int i = 0; i < waySize; i++)
    // {
    //     // (cacheSize, cacheLineSize, nWay)
    //     c.setAssociativeNoAllocOnWriteMiss(16384, 32, ways[i]);
    // }

    // c.setAssociativeNextLinePrefetch(16384, 32, 16);
    //  c.prefetchOnMiss(16384, 32, 2);

    // c.writeFile(outputFileName);

    return 0;
}
