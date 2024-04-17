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
        cerr << "Must have 3 arguments. ./cache-sim inputFileName outputFileName" << endl;
        return 1;
    }
    string inputFileName = argv[1];
    string outputFileName = argv[2];

    // Assume that each cache line has a size of 32 bytes and model the caches sized at 1KB, 4KB, 16KB and 32KB
    int cacheSize[4] = {1024, 4096, 16384, 32768};
    int numCacheSizes = sizeof(cacheSize) / sizeof(cacheSize[0]);

    Cache c;
    c.readFile(inputFileName);

    // (cacheSize, cacheLineSize)
    for (int i = 0; i < numCacheSizes; i++)
    {
        c.directMapped(cacheSize[i], 32);
    }

    c.writeFile(outputFileName);

    return 0;
}
