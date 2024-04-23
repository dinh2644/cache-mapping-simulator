#include <iostream>
#include "Cache.h"
#include <bitset>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>
using namespace std;
unsigned long long totalMemoryAccess;

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
        totalMemoryAccess++;
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

    unsigned int numCacheLines = cacheSize / cacheLineSize;

    vector<unsigned long long> cache(numCacheLines, 0);

    for (int i = 0; i < tracesVect.size(); i++)
    {
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
        unsigned long long tag = -1;
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
        for (int i = 0; i < nWay; i++)
        {
            if (cache[setIndex][i].tag == tag)
            {
                cacheHit++;
                cache[setIndex][i].MRU = memoryAccessed;
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
            for (int i = 1; i < nWay; i++)
            {
                if (cache[setIndex][i].MRU < minMRU)
                {
                    minMRU = cache[setIndex][i].MRU;
                    minIndex = i;
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

        // LRU policy
        if (!found && policy == "LRU")
        {
            unsigned long long minMRU = cache[0].MRU;
            int minIndex = 0;
            // Loop through each line in a set to determine the LRU
            for (int i = 1; i < cache.size(); i++)
            {
                if (cache[i].MRU < minMRU)
                {
                    minMRU = cache[i].MRU;
                    minIndex = i;
                }
            }
            // After determining which of the line is LRU, we'll replace that line with current tag/data
            cache[minIndex].tag = tag;
            cache[minIndex].MRU = memoryAccessed;
        }

        // pLRU Policy
        if (!found && policy == "pLRU")
        {

            int victimIndex = -1;
            currentIndex = 0; // root

            // Find a replacement index when cache MISS occurs by flipping each bit starting from ROOT
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

        else
        {
            currentIndex = hitIndex + (numCacheLines - 1);

            // Flip each bit from the index the cache HIT occured up to ROOT
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

    result.push_back(cacheHit);
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
        unsigned long long tag = -1;
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
        for (int i = 0; i < nWay; i++)
        {
            if (cache[setIndex][i].tag == tag)
            {
                cacheHit++;
                cache[setIndex][i].MRU = memoryAccessed;
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
            for (int i = 1; i < nWay; i++)
            {
                if (cache[setIndex][i].MRU < minMRU)
                {
                    minMRU = cache[setIndex][i].MRU;
                    minIndex = i;
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
        unsigned long long tag = -1;
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
        for (int i = 0; i < nWay; i++)
        {
            if (cache[setIndex][i].tag == tag)
            {
                cacheHit++;
                cache[setIndex][i].MRU = memoryAccessed;
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
            for (int i = 1; i < nWay; i++)
            {
                if (cache[setIndex][i].MRU < minMRU)
                {
                    minMRU = cache[setIndex][i].MRU;
                    minIndex = i;
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
        for (int i = 0; i < nWay; i++)
        {
            if (cache[nextSetIndex][i].tag == nextTag)
            {
                cache[nextSetIndex][i].MRU = memoryAccessed;
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
            for (int i = 1; i < nWay; i++)
            {
                if (cache[nextSetIndex][i].MRU < minMRU)
                {
                    minMRU = cache[nextSetIndex][i].MRU;
                    minIndex = i;
                }
            }
            // After determining which line is LRU, replace that line with the prefetched tag/data
            cache[nextSetIndex][minIndex].tag = nextTag;
            cache[nextSetIndex][minIndex].MRU = memoryAccessed;
        }
    }

    result.push_back(cacheHit);
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
        unsigned long long tag = -1;
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
        for (int i = 0; i < nWay; i++)
        {
            if (cache[setIndex][i].tag == tag)
            {
                cacheHit++;
                cache[setIndex][i].MRU = memoryAccessed;
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
            for (int i = 1; i < nWay; i++)
            {
                if (cache[setIndex][i].MRU < minMRU)
                {
                    minMRU = cache[setIndex][i].MRU;
                    minIndex = i;
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
            for (int i = 0; i < nWay; i++)
            {
                if (cache[nextSetIndex][i].tag == nextTag)
                {
                    cache[nextSetIndex][i].MRU = memoryAccessed;
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
                for (int i = 1; i < nWay; i++)
                {
                    if (cache[nextSetIndex][i].MRU < minMRU)
                    {
                        minMRU = cache[nextSetIndex][i].MRU;
                        minIndex = i;
                    }
                }
                // After determining which line is LRU, replace that line with the prefetched tag/data
                cache[nextSetIndex][minIndex].tag = nextTag;
                cache[nextSetIndex][minIndex].MRU = memoryAccessed;
            }
        }
    }

    result.push_back(cacheHit);
}

void Cache::writeFile(string fileName)
{
    ofstream file(fileName);

    for (int i = 0; i < result.size(); i++)
    {
        if (i >= 0 && i <= 2)
        {
            if (i == 3)
            {
                file << result.at(i) << "," << totalMemoryAccess << ";" << endl;
            }
            file << result.at(i) << "," << totalMemoryAccess << "; ";
        }
        else if (i >= 4 && i <= 6)
        {
            if (i == 7)
            {
                file << result.at(i) << "," << totalMemoryAccess << ";" << endl;
            }
            file << result.at(i) << "," << totalMemoryAccess << "; ";
        }
        else if (i >= 10 && i <= 12)
        {
            if (i == 13)
            {
                file << result.at(i) << "," << totalMemoryAccess << ";" << endl;
            }
            file << result.at(i) << "," << totalMemoryAccess << "; ";
        }
        else if (i >= 14 && i <= 16)
        {
            if (i == 17)
            {
                file << result.at(i) << "," << totalMemoryAccess << ";" << endl;
            }
            file << result.at(i) << "," << totalMemoryAccess << "; ";
        }
        else if (i >= 18 && i <= 20)
        {
            if (i == 21)
            {
                file << result.at(i) << "," << totalMemoryAccess << ";" << endl;
            }
            file << result.at(i) << "," << totalMemoryAccess << "; ";
        }
        else
        {
            file << result.at(i) << "," << totalMemoryAccess << ";" << endl;
        }
    }

    file.close();
}