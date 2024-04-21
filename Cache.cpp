#include <iostream>
#include "Cache.h"
#include <bitset>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>
using namespace std;
#define PAS 32 // (physical address space)
unsigned long long totalTraces;

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
        totalTraces++;
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
        unsigned long long setIndex = (currentAddress / cacheLineSize) % numCacheLines;
        unsigned long long setIndexBit = log2(1);
        unsigned long long byteOffsetBit = log2(cacheLineSize);
        unsigned long long tag = currentAddress >> (setIndexBit + byteOffsetBit);

        if (cache.at(setIndex) == tag)
        {
            cacheHit++;
        }
        else
        {
            cache.at(setIndex) = tag;
        }
    }

    result.push_back(cacheHit);
}

void Cache::setAssociative(int cacheSize, int cacheLineSize, int nWay)
{
    int cacheHit = 0;
    unsigned long long memoryAccessed = 0;

    unsigned int numCacheLines = cacheSize / cacheLineSize;
    unsigned int numOfSets = numCacheLines / nWay;

    // Give each Way(set) its own MRU counter to keep track whuich was used the most recently
    struct Way
    {
        unsigned long long MRU = 0;
        unsigned long long tag = 0;
    };

    // initialize a cache with numOfSets sets where each set contains nWay lines
    vector<vector<Way>> cache(numOfSets, vector<Way>(nWay));

    for (int i = 0; i < tracesVect.size(); i++)
    {
        memoryAccessed++;
        unsigned long long currentAddress = tracesVect.at(i).getByteMemAddr();

        // Dissect current address into 3 parts (tag,index,offset)
        unsigned long long setIndex = ((currentAddress / cacheLineSize) % numOfSets);
        unsigned long long setIndexBit = log2(nWay);
        unsigned long long byteOffsetBit = log2(cacheLineSize);
        unsigned long long tag = currentAddress >> (setIndexBit + byteOffsetBit);

        bool found = false;
        // loop through each lines in the current set nWay times
        for (int j = 0; j < nWay; j++)
        {
            if (cache[setIndex][j].tag == tag)
            {
                cacheHit++;
                cache[setIndex][j].MRU = memoryAccessed;
                found = true;
                break;
            }
        }

        // Replacement policy
        if (!found)
        {
            unsigned long long minMRU = cache[setIndex][0].MRU;
            int minIndex = 0;
            // Loop through each line in a set to determine the LRU
            for (int k = 1; k < nWay; k++)
            {
                if (cache[setIndex][k].MRU < minMRU)
                {
                    minMRU = cache[setIndex][k].MRU;
                    minIndex = k;
                }
            }
            // After determining which of the line is LRU, we'll replace that line with current tag/data
            cache[setIndex][minIndex].tag = tag;
            cache[setIndex][minIndex].MRU = memoryAccessed;
        }
    }

    result.push_back(cacheHit);
}

void Cache::fullyAssociative(int cacheSize, int cacheLineSize)
{
    int cacheHit = 0;
    unsigned long long memoryAccessed = 0;

    unsigned int numCacheLines = cacheSize / cacheLineSize;

    struct Way
    {
        unsigned long long MRU = 0;
        unsigned long long tag = 0;
        unsigned long long hot_cold_bit = 0;
    };

    // initialize a cache with numOfSets sets where each set contains nWay lines
    vector<Way> cache(numCacheLines);

    for (int i = 0; i < tracesVect.size(); i++)
    {
        memoryAccessed++;
        unsigned long long currentAddress = tracesVect.at(i).getByteMemAddr();

        // Dissect current address into 3 parts (tag,index,offset)
        unsigned long long setIndex = ((currentAddress / cacheLineSize) % numCacheLines);
        unsigned long long setIndexBit = log2(1);
        unsigned long long byteOffsetBit = log2(cacheLineSize);
        unsigned long long tag = currentAddress >> (setIndexBit + byteOffsetBit);

        bool found = false;
        // loop through any slot in cache to check for cache hit or to insert it
        for (int j = 0; j < numCacheLines; j++)
        {
            if (cache[j].tag == tag)
            {
                cacheHit++;
                cache[j].MRU = memoryAccessed;
                found = true;
                break;
            }
        }

        // LRU policy
        if (!found)
        {
            unsigned long long minMRU = cache[0].MRU;
            int minIndex = 0;
            // Loop through each line in a set to determine the LRU
            for (int k = 1; k < cache.size(); k++)
            {
                if (cache[k].MRU < minMRU)
                {
                    minMRU = cache[k].MRU;
                    minIndex = k;
                }
            }
            // After determining which of the line is LRU, we'll replace that line with current tag/data
            cache[minIndex].tag = tag;
            cache[minIndex].MRU = memoryAccessed;
        }

        // LFU policy (hot cold)
        // if (!found)
        // {
        //     // loop trough each slot from beginning to check for any immediate open slot
        //     for (int j = 0; j < cache.size(); j++)
        //     {
        //         if (cache[i].tag == 0)
        //         {
        //             cache[i].tag = tag;
        //         }
        //     }
        // }
    }

    cout << cacheHit << "," << memoryAccessed << endl;
    //  result.push_back(cacheHit);
}

void Cache::writeFile(string fileName)
{
    ofstream file(fileName);

    for (int i = 0; i < result.size(); i++)
    {
        if (i >= 0 && i <= 4)
        {
            file << result.at(i) << "," << totalTraces << ";" << endl;
        }
    }

    file.close();
}