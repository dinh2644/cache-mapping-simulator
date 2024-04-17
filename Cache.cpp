#include <iostream>
#include "Cache.h"
#include <bitset>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>
using namespace std;
#define PAS 32 // (physical address space)
unsigned long long totalBranches;

void Cache::readFile(string fileName)
{
    // Temporary variables
    string flag, line;
    unsigned long long byteMemAddr;

    // Open file for reading
    ifstream infile(fileName);
    if (!infile)
    {
        cout << "No file found!" << endl;
        exit(1);
    }

    // The following loop will read a line at a time
    while (getline(infile, line))
    {
        totalBranches++;
        // Now we have to parse the line into it's two pieces
        stringstream s(line);
        s >> flag >> std::hex >> byteMemAddr;
        // Now we can output it
        if (flag == "L")
        {
            Trace trace(0, byteMemAddr);
            tracesVect.push_back(trace);
        }
        else if (flag == "S")
        {
            Trace trace(1, byteMemAddr);
            tracesVect.push_back(trace);
        }
        else
        {
            cout << "Unidentifiable behavior" << endl;
            exit(1);
        }
    }
}

void Cache::directMapped(int cacheSize, int cacheLineSize)
{
    int cacheHit = 0;
    int memoryAccessed = 0;

    unsigned int numCacheLines = cacheSize / cacheLineSize;

    vector<unsigned long long> cache(numCacheLines, 0);

    for (int i = 0; i < tracesVect.size(); i++)
    {
        memoryAccessed++;
        unsigned long long currentAddress = tracesVect.at(i).getByteMemAddr();

        // Dissect current address into 3 parts (tag,index,offset)
        // Byte offset not needed
        unsigned long long setIndex = (currentAddress / cacheLineSize) % numCacheLines;
        unsigned long long tag = currentAddress / cacheSize;

        if (cache.at(setIndex) == tag)
        {
            cacheHit++;
        }
        else
        {
            cache.at(setIndex) = tag;
        }
    }

    cout << cacheHit << "," << memoryAccessed << ";" << endl;
}

void Cache::setAssociative(int cacheSize, int cacheLineSize)
{
    int cacheHit = 0;
    int memoryAccessed = 0;

    unsigned int numCacheLines = cacheSize / cacheLineSize;

    vector<unsigned long long> cache(numCacheLines, 0);

    for (int i = 0; i < tracesVect.size(); i++)
    {
        memoryAccessed++;
        unsigned long long currentAddress = tracesVect.at(i).getByteMemAddr();

        // Dissect current address into 3 parts (tag,index,offset)
        // Byte offset not needed
        unsigned long long setIndex = (currentAddress / cacheLineSize) % numCacheLines;
        unsigned long long tag = currentAddress / cacheSize;

        // logic here
    }

    cout << cacheHit << "," << memoryAccessed << ";" << endl;
}