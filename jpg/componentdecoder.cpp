#include "componentdecoder.h"
#include <algorithm>

namespace jpg{


ComponentDecoder::ComponentDecoder(DHT &dht):
 _values(dht.symbols)
{
    this->_lastDc = 0;

    this->generateSymbols(&(dht.countHuffmanCodes));

    this->_max.resize(dht.countHuffmanCodes.size());
    this->_min.resize(dht.countHuffmanCodes.size());
    this->_firstIndex.resize(dht.countHuffmanCodes.size());

    this->findMinMax();
}

 ComponentDecoder::~ComponentDecoder()
{

}

char ComponentDecoder::decodeNext(BitReader &reader)
{
    int code = 0;
    int indexValue = -1;


    for(int codelen = 0; codelen < 16; codelen++){
        //read next bit in stream
        char newBit = reader.readNexBit();

		
        code = code << 1;
        code = code | newBit;


        if(code <= _max[codelen]){
            indexValue = _firstIndex[codelen] + code - _min[codelen];
            break;
        }
    }

    if(indexValue == -1){
        throw std::logic_error("error al leer");
    }

    char value = _values[indexValue];

    return value;
}


void ComponentDecoder::generateTables()
{

}

void ComponentDecoder::generateSymbols(vector<char> *countsHuffman)
{
        for(size_t i = 0; i < countsHuffman->size(); i++){
            size_t count = (unsigned char)(*countsHuffman)[i];

            for(size_t j = 0; j < count; j++){
                    HuffmanSymbol symbol;
                    symbol.codeLength = i + 1;
                    _symbols.push_back(symbol);
            }
        }

        this->generateCodes(_symbols, false);

}

void ComponentDecoder::generateCodes(vector<HuffmanSymbol> &codelen, bool order)
{
    if(order == true){
        std::sort(codelen.begin(), codelen.end(),
                  [](const HuffmanSymbol &e, const HuffmanSymbol &e2){
                    return e.codeLength < e2.codeLength;
        });
    }


    size_t index = 0, len = codelen.size();
    int huffmancodecounter = 0;
    int codelengthcounter = 1;

    while(index < len){
        HuffmanSymbol &symbol = codelen[index];

        if(symbol.codeLength == codelengthcounter){
            symbol.code = huffmancodecounter;
            huffmancodecounter++;
            index++;

        }else{
            huffmancodecounter = huffmancodecounter << 1;
            codelengthcounter++;
        }
    }

}

void ComponentDecoder::findMinMax()
{
    size_t indexSimbol = 0;
    int lowest = std::numeric_limits<int>::min();
    int highest = std::numeric_limits<int>::max();


    for(int len = 1; len <= this->_max.size(); len++){

        int max = lowest;
        int min = highest;
        int numSymbolsByLen = 0;
        int firstIndex = -1;

		while (indexSimbol < this->_symbols.size() && this->_symbols[indexSimbol].codeLength == len){

            if(firstIndex == -1)
                firstIndex = indexSimbol;

            if(_symbols[indexSimbol].code > max)
                max = _symbols[indexSimbol].code;

            if(_symbols[indexSimbol].code < min)
                min = _symbols[indexSimbol].code;


            numSymbolsByLen++;
            indexSimbol++;
        }

        if(numSymbolsByLen <= 0){
            max = -1;
            min = -1;
        }

        _max[len - 1] = max;
        this->_min[len - 1] = min;
        this->_firstIndex[len -1] = firstIndex;
    }

}

char ComponentDecoder::decodeDC(BitReader &reader)
{
    unsigned char magnitude = this->decodeNext(reader);
    auto additional = reader.readBits(magnitude);

    auto difference = this->extend(additional, magnitude);

    auto dc = difference - this->_lastDc;
    this->_lastDc = dc;

    return dc;

}

int ComponentDecoder::extend(unsigned char additional, unsigned char magnitude)
{
    unsigned char vt = 1 << (magnitude - 1);

    if(additional < vt){
        int vv = ( -1 << magnitude ) + 1;
        return additional + vv;

    }else{
        return additional;
    }

}

char ComponentDecoder::decodeAC(char dc, BitReader &reader)
{	
	const std::size_t size = 64;

	const unsigned int zigZagInput[size] =
	{
		0, 1, 8, 16, 9, 2, 3, 10,
		17, 24, 32, 25, 18, 11, 4, 5,
		12, 19, 26, 33, 40, 48, 41, 34,
		27, 20, 13, 6, 7, 14, 21, 28,
		35, 42, 49, 56, 57, 50, 43, 36,
		29, 22, 15, 23, 30, 37, 44, 51,
		58, 59, 52, 45, 38, 31, 39, 46,
		53, 60, 61, 54, 47, 55, 62, 63
	};
	char coef[size];

	for (auto j = 0; j < size; j++)
		coef[j] = 0;
	

	coef[0] = dc;

	std::size_t i = 1;

	while (i < size){
		unsigned char ac = this->decodeNext(reader);
		unsigned char zeroCount = ac >> 4;
		unsigned char magnitude = ac & 0xF;

		if (ac == 0x00)
			i = 64;//if 0x00 all remaining are 0, finish loop
		else if (ac == 0xF0)
			i += 16;//next sixteen are 0
		else{
			i = i + zeroCount;//skipped zeros

			auto readBits = reader.readBits(magnitude);
			auto coefAc = this->extend(readBits, magnitude);
			auto pos = zigZagInput[i];

			coef[pos] = (char)coefAc;
			i = i + 1;
		}
	}

}


}
