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

void Cache::fullyAssociative(int cacheSize, int cacheLineSize, string policy)
{
    int cacheHit = 0;
    unsigned long long memoryAccessed = 0;

    unsigned int numCacheLines = cacheSize / cacheLineSize;

    struct Way
    {
        unsigned long long MRU = 0;
        unsigned long long tag = -1;
        unsigned int hotColdBit = 0;
    };

    // Cache initialization
    vector<Way> cache(numCacheLines);

    for (int i = 0; i < tracesVect.size(); i++)
    {
        memoryAccessed++;
        unsigned long long currentAddress = tracesVect.at(i).getByteMemAddr();

        // Dissect current address into tag and index
        unsigned long long setIndexBit = log2(1);
        unsigned long long byteOffsetBit = log2(cacheLineSize);
        unsigned long long tag = currentAddress >> (setIndexBit + byteOffsetBit);

        bool found = false;
        int hitIndex = -1;
        int currentIndex = -1;

        for (int i = 0; i < numCacheLines; i++)
        {
            if (cache[i].tag == tag)
            {
                cacheHit++;
                cache[i].MRU = memoryAccessed;
                found = true;
                hitIndex = i;
                break;
            }
        }

        // pLRU Policy
        // Find a replacement index when cache miss occurs by flipping each bit starting from ROOT
        if (!found)
        {

            int victimIndex = -1;
            currentIndex = 0; // root

            for (int i = 0; i < log2(numCacheLines); i++) // log2(numCacheLines) = depth of tree
            {
                // IF CURRENT BIT 0, FLIP TO 1
                if (cache[currentIndex].hotColdBit == 0)
                {
                    cache[currentIndex].hotColdBit = 1;
                    // TRAVERSE TO RIGHT CHILD
                    currentIndex = 2 * currentIndex + 2;
                }
                // IF CURRENT BIT 1, FLIP TO 0
                else
                {
                    cache[currentIndex].hotColdBit = 0;
                    // TRAVERSE TO LEFT CHILD
                    currentIndex = 2 * currentIndex + 1;
                }
            }

            victimIndex = currentIndex - (numCacheLines - 1);
            cache[victimIndex].tag = tag;
            cache[victimIndex].MRU = memoryAccessed;
        }
        // Flip each bit from the index the cache hit occured up to ROOT
        else
        {
            currentIndex = hitIndex + (numCacheLines - 1);
            for (int i = 0; i < log2(numCacheLines); i++) // log2(numCacheLines) = depth of tree
            {
                if (currentIndex % 2 == 0)
                {
                    // TRAVERSE TO PARENT NODE
                    currentIndex = (currentIndex - 1) / 2;
                    // FLIP BIT
                    cache[currentIndex].hotColdBit = 1;
                }
                else
                {
                    // TRAVERSE TO PARENT NODE
                    currentIndex = (currentIndex - 1) / 2;
                    // FLIP BIT
                    cache[currentIndex].hotColdBit = 0;
                }
            }
        }
    }

    cout << cacheHit << "," << memoryAccessed << endl;
}

void Cache::setAssociativeNoAllocOnWriteMiss(int cacheSize, int cacheLineSize, int nWay)
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
        int currentFlag = tracesVect.at(i).getFlag();

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
        if (!found && currentFlag != 1)
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

void Cache::setAssociativeNextLinePrefetch(int cacheSize, int cacheLineSize, int nWay)
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

        // Next line prefetching
        unsigned long long nextAddress = currentAddress + cacheLineSize;
        unsigned long long nextSetIndex = ((nextAddress / cacheLineSize) % numOfSets);
        unsigned long long nextTag = nextAddress >> (setIndexBit + byteOffsetBit);

        bool nextFound = false;
        // loop through each lines in the current set nWay times
        for (int j = 0; j < nWay; j++)
        {
            if (cache[nextSetIndex][j].tag == nextTag)
            {
                cache[nextSetIndex][j].MRU = memoryAccessed;
                nextFound = true;
                break;
            }
        }

        // Replacement policy
        if (!nextFound)
        {
            unsigned long long minMRU = cache[nextSetIndex][0].MRU;
            int minIndex = 0;
            // Loop through each line in a set to determine the LRU
            for (int k = 1; k < nWay; k++)
            {
                if (cache[nextSetIndex][k].MRU < minMRU)
                {
                    minMRU = cache[nextSetIndex][k].MRU;
                    minIndex = k;
                }
            }
            // After determining which line is LRU, replace that line with the prefetched tag/data
            cache[nextSetIndex][minIndex].tag = nextTag;
            cache[nextSetIndex][minIndex].MRU = memoryAccessed;
        }
    }

    cout << cacheHit << "," << memoryAccessed << endl;
}

void Cache::prefetchOnMiss(int cacheSize, int cacheLineSize, int nWay)
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

            // Next line prefetching
            unsigned long long nextAddress = currentAddress + cacheLineSize;
            unsigned long long nextSetIndex = ((nextAddress / cacheLineSize) % numOfSets);
            unsigned long long nextTag = nextAddress >> (setIndexBit + byteOffsetBit);

            bool nextFound = false;
            // loop through each lines in the current set nWay times
            for (int j = 0; j < nWay; j++)
            {
                if (cache[nextSetIndex][j].tag == nextTag)
                {
                    cache[nextSetIndex][j].MRU = memoryAccessed;
                    nextFound = true;
                    break;
                }
            }

            // Replacement policy
            if (!nextFound)
            {
                unsigned long long minMRU = cache[nextSetIndex][0].MRU;
                int minIndex = 0;
                // Loop through each line in a set to determine the LRU
                for (int k = 1; k < nWay; k++)
                {
                    if (cache[nextSetIndex][k].MRU < minMRU)
                    {
                        minMRU = cache[nextSetIndex][k].MRU;
                        minIndex = k;
                    }
                }
                // After determining which line is LRU, replace that line with the prefetched tag/data
                cache[nextSetIndex][minIndex].tag = nextTag;
                cache[nextSetIndex][minIndex].MRU = memoryAccessed;
            }
        }
    }

    cout << cacheHit << "," << memoryAccessed << endl;
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