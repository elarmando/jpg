#ifndef COMPONENTDECODER_H
#define COMPONENTDECODER_H

#include "huffmancoding.h"
#include "bitreader.h"
#include "reader.h"

using namespace std;
namespace jpg{


class ComponentDecoder
{

private:
    vector<char> _values;
    vector<HuffmanSymbol> _symbols;
    vector<int> _max;
    vector<int> _min;
    vector<int> _firstIndex;

    int _lastDc;

    void generateTables();
    void generateSymbols( vector<char> *counts);
    void generateCodes(vector<HuffmanSymbol> &, bool order = false);
    void findMinMax();



public:
    ComponentDecoder(DHT &huffmanTable);
    ~ComponentDecoder();

    char decodeDC(BitReader &reader);
    char decodeNext(BitReader &reader);
	void decodeAC(char dc, BitReader &reader, std::vector<char> &coefficients);
    int extend(unsigned char additional, unsigned char magnitude);

};
}

#endif // COMPONENTDECODER_H
