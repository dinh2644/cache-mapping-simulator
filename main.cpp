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
    int numCacheSizes = sizeof(cacheSize) / sizeof(cacheSize[0]);

    Cache c;
    c.readFile("project2/traces/trace1.txt");

    // (cacheSize, cacheLineSize)
    // for (int i = 0; i < numCacheSizes; i++)
    // {
    //     c.directMapped(cacheSize[i], 32);
    // }

    c.setAssociative(16384, 32, 4);

    // c.writeFile(outputFileName);

    return 0;
}
