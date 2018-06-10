#include "huffmancoding.h"

#include "huffmancoding.h"
#include <string>
#include <sstream>
#include <iostream>
#include <limits>
#include <algorithm>
#include <limits>

using namespace std;

namespace  jpg{
HuffmanSymbol::HuffmanSymbol(char _symbol, int _codeLength):

symbol(_symbol),
codeLength(_codeLength),
code(0)
{

}

 HuffmanSymbol::HuffmanSymbol():
 symbol('\0'),
 codeLength(0),
   code(0)
 {

 }

string HuffmanSymbol::toStr(){
    stringstream stream;
    stream <<"(" <<symbol <<", " <<codeLength <<")";
    return stream.str();
}

HuffmanSymbols::HuffmanSymbols():
frequency(0)
{

}

string HuffmanSymbols::toStr(){
    vector<HuffmanSymbol>::iterator ite = nodes.begin();
    stringstream stream;

    while( ite != nodes.end()){
        HuffmanSymbol &node = *ite;
        stream << node.toStr()<<"  ";


        ite++;
    }

    stream<<"freq " << frequency;

    return stream.str();

}

void HuffmanCoding::generateInitialList(vector<HuffmanSymbols> &lista)
{

    for(size_t i = 0; i < symbols.size(); i++){
        HuffmanSymbols list;
        HuffmanSymbol symbol(symbols[i], 0);

        list.nodes.push_back(symbol);
        list.frequency = frequencies[i];

        lista.push_back(list);

    }

}

void HuffmanCoding::generateCodes(HuffmanSymbol &simbols)
{

}

HuffmanCoding::HuffmanCoding(vector<char> _symbols, vector<int> _frequencies):
symbols(_symbols),
frequencies(_frequencies)
{

}

vector<long> HuffmanCoding::getTwoLowestFreq(vector<HuffmanSymbols> &lista)
{
    long first = this->getLowestFreq(lista);
    long second = this->getLowestFreq(lista, first);

    vector<long> lowest;
    lowest.push_back(first);
    lowest.push_back(second);


    return lowest;
}

long HuffmanCoding::getLowestFreq(vector<HuffmanSymbols> &lista, long indexToSkip)
{
    unsigned int minVal = std::numeric_limits<unsigned long>::max();
    unsigned int currentVal;
    long lowestIndex = -1;
    long l = lista.size();

    for(long i = 0; i < l; i++){
        currentVal = lista[i].frequency;

        if(currentVal <= minVal && indexToSkip != i){
            lowestIndex = i;
            minVal = currentVal;
        }

    }

    return lowestIndex;

}

void HuffmanCoding::generateCodeLengths(vector<HuffmanSymbols> &e)
{
    while(e.size() > 1){
        vector<long> indexes = this->getTwoLowestFreq(e);

        HuffmanSymbols &simbols1 = e[indexes[0]];
        HuffmanSymbols &simbols2 = e[indexes[1]];

        //new list frequency is the sum of the previous ones
        simbols1.frequency += simbols2.frequency;


        //add each simbol of the second list to the first one
        auto ite = simbols2.nodes.begin();
        auto end = simbols2.nodes.end();

        while(ite != end){
            simbols1.nodes.push_back(*ite);
            ite++;
        }

        //increment all code lengths
        ite = simbols1.nodes.begin();
        end = simbols1.nodes.end();

        while(ite != end){
            HuffmanSymbol &symbol = *ite;
            symbol.codeLength++;
            ite++;
        }

        //removes second of the list
        e.erase(e.begin() + indexes[1]);

    }

}

void HuffmanCoding::generateCodes(vector<HuffmanSymbol> &codelen, bool order )
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

void HuffmanCoding::generateCodeLengthsFromCounts(vector<char> &counts, vector<HuffmanSymbol> &outLengths, bool resize)
{
    if(resize){
        size_t len = 0;

       auto end = counts.end();

       for(auto ite = counts.begin(); ite != end; ite++)
           len += (unsigned char)(*ite);

        outLengths.resize(len);
    }

    size_t index = 1;
    for(size_t i = 1; i < counts.size(); i+=1){
        unsigned char count = counts[i];

        for(size_t j = 0; j < count; j++){
            outLengths[index].codeLength = index;
            index++;

        }
    }

}



void HuffmanCoding::readStream(std::vector<char> &countHuffman, std::vector<char> &values, std::vector<char> stream, vector<char> &outStream){

   vector<HuffmanSymbol> simbols;

   for(size_t i = 0; i < countHuffman.size(); i++){
       size_t count = (unsigned char)countHuffman[i];

       for(size_t j = 0; j < count; j++){
               HuffmanSymbol symbol;
               symbol.codeLength = i + 1;
               simbols.push_back(symbol);
       }
   }

   this->generateCodes(simbols, false);

    std::vector<int> maxCode, minCode, firstIndexVal;
    maxCode.resize(countHuffman.size());
    minCode.resize(countHuffman.size());
    firstIndexVal.resize(countHuffman.size());


    findMinMax(simbols, maxCode, minCode, firstIndexVal);

    unsigned long indexBit = 0;
    unsigned long endBit = 8 * stream.size();

    while(indexBit < endBit){


        int code = 0;
        int indexValue = -1;


        for(int codelen = 1; codelen <= 16; codelen++){
            //read next bit in stream
            int newBit = this->readBit(indexBit, stream);
            indexBit++;

            code = code << 1;
            code = code || newBit;


            if(code <= maxCode[codelen]){
                indexValue = firstIndexVal[codelen] + code - minCode[codelen];
                break;
            }
        }

        if(indexValue == -1){
            throw std::logic_error("error al leer");
        }

        char value = values[indexValue];
        outStream.push_back(value);

    }
}

int HuffmanCoding::readBit(unsigned long indexBit, vector<char> &stream){
    int bitPos = indexBit % 8;
    char byte = stream[indexBit / 8];

    return (byte >> (8 - bitPos - 1)) && (0x1);
}

void HuffmanCoding::findMinMax(std::vector<HuffmanSymbol> &symbols,
                                std::vector<int> &maxCode,
                                std::vector<int> &minCode,
                                vector<int> &firstIndexValue){

    size_t indexSimbol = 0;
    int lowest = std::numeric_limits<int>::min();
    int highest = std::numeric_limits<int>::max();


    for(int len = 1; len <= maxCode.size(); len++){

        int max = lowest;
        int min = highest;
        int numSymbolsByLen = 0;
        int firstIndex = -1;

        while(symbols[indexSimbol].codeLength == len){

            if(firstIndex == -1)
                firstIndex = indexSimbol;

            if(symbols[indexSimbol].code > max)
                max = symbols[indexSimbol].code;

            if(symbols[indexSimbol].code < min)
                min = symbols[indexSimbol].code;


            numSymbolsByLen++;
            indexSimbol++;
        }

        if(numSymbolsByLen <= 0){
            max = -1;
            min = -1;
        }

        maxCode[len - 1] = max;
        minCode[len - 1] = min;
        firstIndexValue[len -1] = firstIndex;
    }

}


void HuffmanCoding::generateCodes(){

    vector<HuffmanSymbols> lista;
    this->generateInitialList(lista);
}




}
