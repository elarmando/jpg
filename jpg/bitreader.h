#ifndef BITREADER_H
#define BITREADER_H

#include <iostream>
#include <vector>


using namespace std;

namespace jpg{
class BitReader
{
private:

    size_t _bitPos;
    const vector<char>* const _buffer;

public:
    BitReader(const vector<char>* const buffer);

    char readNexBit();
};
}
#endif // BITREADER_H
