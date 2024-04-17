#include <iostream>
#include "Cache.h"
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

int main(int argc, char const *argv[])
{
    Cache c;
    c.readFile("project2/traces/trace1.txt");

    // (cacheSize, cacheLineSize)
    c.directMapped(1024, 32);
    return 0;
}
