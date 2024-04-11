#include <iostream>
using namespace std;

/**
    Class to represent each trace line
    flag (load/store), byte memory address
        (i.e. S 0x0022f5b4)
**/
class Trace
{
private:
    int flag;
    unsigned long long byteMemAddr;

public:
    // Constructor
    Trace(int flag, unsigned long long byteMemAddr) : flag(flag), byteMemAddr(byteMemAddr){};

    // Getters
    int getFlag() const { return flag; }
    unsigned long long getByteMemAddr() const { return byteMemAddr; }
};