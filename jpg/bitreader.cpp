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
    char byte = (*_buffer)[this->_bitPos / 8];

    this->_bitPos++;

    return (byte >> (8 - bitPos - 1)) && (0x1);
}

}
