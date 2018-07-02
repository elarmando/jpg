#include "bitreader.h"
namespace jpg{


BitReader::BitReader(const vector<char>* const buf):
    _buffer(buf),
    _bitPos(0)

{

}

char BitReader::readNexBit()
{
    int bitPos = this->_bitPos % 8;
    unsigned char byte = (*_buffer)[this->_bitPos / 8];

    this->_bitPos++;

    unsigned int d = (byte >> (8 - bitPos - 1));
    unsigned int f = 0x1;

    return d & f;
}

unsigned char BitReader::readBits(unsigned char numberOfBits)
{
    unsigned char result  = 0;

    for(size_t i = 0; i < numberOfBits; i++){
        result = result << 1;
        auto e = (unsigned char)this->readNexBit();
        result = result | e;
    }


    return result;

}



}
