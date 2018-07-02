#include "componentdecoder.h"
#include <algorithm>

namespace jpg{


ComponentDecoder::ComponentDecoder(DHT &dht):
 _values(dht.symbols)
{

    std::cout<<dht.countHuffmanCodes.size()<<std::endl;

    std::vector<char> p;

    p.push_back('a');

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


    for(int codelen = 1; codelen <= 16; codelen++){
        //read next bit in stream
        char newBit = reader.readNexBit();


        code = code << 1;
        code = code || newBit;


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

    cout<<"hola";
    cout<<countsHuffman->size()<<std::endl;

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

        while(this->_symbols[indexSimbol].codeLength == len){

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
    auto magnitude = this->decodeNext(reader);

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

}
