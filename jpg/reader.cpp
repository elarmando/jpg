#include "reader.h"

#include <istream>
#include <iostream>
#include <fstream>
#include <stdexcept>

#include "bitreader.h"
#include "componentdecoder.h"

namespace jpg{

DQT::DQT(){

}

ComponentSOF::ComponentSOF(){}

SOF0::SOF0(){}


SOSComponentDescriptor::SOSComponentDescriptor()
{

}

SOS::SOS()
{}

reader::reader(const std::string &a):
    pathFile(a),
    _stream(pathFile.c_str(), std::ios::binary)
{
   // _stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

}

reader::~reader()
{

}

void reader::skipAppMarkers()
{
    uint2 marker = 0;
    uint2 length = 0;

    read2bytes(_stream, marker);

    while(Markers::isAPP(marker)){
        read2bytes(_stream, length);
        length -=2;
        _stream.seekg( length, std::ios_base::cur);
        read2bytes(_stream, marker);

    }

    _stream.seekg(-2, std::ios::cur);
}

void reader::readSequenceOfDQT(std::vector<DQT> &dqts, bool skipmarker)
{
    uint2 marker = 0, len = 0;


    if(!skipmarker){
        read2bytes(_stream, marker);

        if(Markers::DQT != marker)
            return;
    }


    read2bytes(_stream, len);
    unsigned long bytesRead = 2;//counting the  2 bytes length

    while(bytesRead < len){

        DQT dqt;
        char byte, identifier, size;
        int tableLength = 0;

        _stream.read(&byte, 1);

        identifier = 0x0F & byte;
        size = (0xF0 & byte)>>4;

        if(size == 0){
            size = 1;
            tableLength = 64;
        }
        else{
            size = 2;
            tableLength = 128;
        }


        dqt.table.resize(tableLength);

        _stream.read(&dqt.table[0], tableLength);

        dqt.identifier = identifier;
        dqt.size = size;

        dqts.push_back(dqt);

        bytesRead+= tableLength + 1;//el tamaño de la tabla y un byte de la descripcion
    }



}

void reader::readDQT(std::vector<DQT> dqts)
{
    uint2 marker = 0;

    read2bytes(_stream, marker);


    while(marker == Markers::DQT){
        readSequenceOfDQT(dqts, true);
        read2bytes(_stream, marker);
    }


    _stream.seekg(-2, std::ios_base::cur);
}

void reader::readSOF0(SOF0 &sof0)
{
    uint2 marker = 0;

    read2bytes(_stream, marker);

    if(marker != Markers::SOF0)
        return;

    uint2 len = 0 ;

    read2bytes(_stream, len);


    _stream.read(&sof0.precision, 1);
    read2bytes(_stream, sof0.height);
    read2bytes(_stream, sof0.width);
    _stream.read(&(sof0.numComponents), 1);

    sof0.components.resize(0);


    for(int i = 0; i < sof0.numComponents; i+=1){
        ComponentSOF component;
        char sampling;

        _stream.read(&component.identifier, 1);
        _stream.read(&sampling, 1);
        _stream.read(&component.quantizationIdentifier, 1);

        component.horizontalSampling = (unsigned char)(sampling & 0xF0) >>4;
        component.verticalSampling = (unsigned char)(sampling & 0x0F);

        _sof0.components.push_back(component);

    }
}

void reader::readSOS(SOS &sos)
{
    uint2 marker = 0, len = 0;

    read2bytes(_stream, marker);

    if(Markers::SOS != marker){
        return;
    }

    read2bytes(_stream, len);



    _stream.read(&sos.componentCount, 1);

    sos.componentDescriptors.resize(sos.componentCount);

    for(char i = 0; i < sos.componentCount; i++){
        SOSComponentDescriptor descriptor;
          char descriptor1 = 0, descriptor2 = 0;
          _stream.read(&descriptor1, 1);
          _stream.read(&descriptor2, 1);

          descriptor.componentIdentifier  = descriptor1;
          descriptor.acHuffmanTable =  (descriptor2 & 0x0F);
          descriptor.dcHuffmanTable = (descriptor2 & 0xF0) >> 4;
    }


    _stream.read(&sos.spectralSelectionStart, 1);
    _stream.read(&sos.spectralSelectionStop, 1);
    _stream.read(&sos.succesiveApproximation, 1);


    auto initilPos = _stream.tellg();
    size_t lenData = 0;

    uint2 newmarker = 0xFF00;
     read2bytes(_stream, newmarker);

    while(!Markers::isMarker(newmarker) || Markers::isRST(newmarker)){//itera mientras no sea un marcador o si es Restart marker
        lenData+=2;

        read2bytes(_stream, newmarker);

    }



    if(lenData > 0){
        sos.dataScan.resize(lenData);

        _stream.seekg(initilPos);
        _stream.read(&sos.dataScan[0], lenData);
    }else{
        sos.dataScan.resize(0);
        _stream.seekg(-2,std::ios_base::cur);
    }



}

void reader::readDHT(DHT &dht, bool skipmarker)
{

    if(skipmarker == false){
        uint2 marker = 0;

        read2bytes(_stream, marker);

        if(marker != Markers::DHT)
            return;
    }



    uint2 len = 0;

    read2bytes(_stream, len);

    char firstByte;

    _stream.read(&firstByte, 1);
    dht.classNum = (firstByte & 0xF0) >> 4;
    dht.identifier = firstByte & 0x0F;

    dht.countHuffmanCodes.resize(16);

    _stream.read(&dht.countHuffmanCodes[0], 16);

    unsigned int sum = 0;

    auto ite = dht.countHuffmanCodes.begin();
    auto end = dht.countHuffmanCodes.end();

    for(;ite != end; ite++){

        char count = *ite;

        sum+= (unsigned char) count;

    }

    dht.symbols.resize(sum);

    _stream.read(dht.symbols.data(), sum);

}

void reader::readDHT(std::vector<DHT> &dhts)
{

    uint2 marker = 0;

    read2bytes(_stream, marker);

    while(marker == Markers::DHT){

        DHT dht;
        readDHT(dht, true);

        dhts.push_back(dht);

        read2bytes(_stream, marker);
    }

    _stream.seekg(-2, std::ios::cur);

}

void reader::validateFile(istream &stream)
{
    uint2 marker;
    read2bytes(stream, marker);

    if(marker != Markers::SOI)
        throw std::logic_error("No jfif file in header");


    read2bytes(stream, marker);

    if(marker != Markers::APP0)
        throw std::logic_error("No app0 marker in header");

    read2bytes(stream, marker);

    char jfifstring[5];

    stream.read(jfifstring, 5);

    if(stream.gcount() < 5)
         throw std::logic_error("No jfif string in header");

    if(jfifstring[0] != 'J' || jfifstring[1] != 'F' ||
       jfifstring[2] != 'I' ||  jfifstring[3] != 'F' ||  jfifstring[4] != 0)
         throw std::logic_error("No jfif string in header");


}

void reader::read()
{
    this->readHeader();
    this->skipAppMarkers();
    this->readDQT(this->_dqts);


    this->readSOF0(this->_sof0);





    uint2 nextMarker = 0;
    read2bytes(this->_stream, nextMarker);

    while(nextMarker != Markers::EOI){

        _stream.seekg(-2,std::ios_base::cur);

        this->readDHT(this->_dhts);
        this->readSOS(this->_sos);

        this->decode(this->_sof0, this->_sos, this->_dhts);

         read2bytes(this->_stream, nextMarker);
    }




}



 void reader::decode(SOF0 &sof, SOS &sos, vector<DHT> &dht )
 {
     unsigned char maxSamplingH = 0;
     unsigned char maxSamplingV = 0;
     unsigned char numComponents = sof.components.size();
     uint2 width = sof.width;
     uint2 height = sof.height;

    this->getMaxSampling(sof, maxSamplingH, maxSamplingV);

    unsigned int numMCUx = 0;
    unsigned int numMCUy = 0;

    this->getNumMCU(sof, maxSamplingH, maxSamplingV, numMCUx, numMCUy);

    std::vector<ComponentSOF> components;

    components.resize(sof.components.size() + 1);

    for(auto it = sof.components.begin(); it != sof.components.end(); ++it){
        auto c = (*it);


        components[c.identifier] = c;
    }


    BitReader bitreader(&(sos.dataScan));

    auto &compDesc = sos.componentDescriptors;

    //decode each data unit in the mcu, for each component
    for(unsigned int colMCU = 0; colMCU < numMCUx; colMCU++){

        for(unsigned int rowMCU = 0; rowMCU < numMCUy; rowMCU++){

            for(auto it = compDesc.begin(); it != compDesc.end(); ++it)
            {
                auto des = (*it);
                auto component = components[des.componentIdentifier];
                int samplingx = component.horizontalSampling;
                int samplingy = component.verticalSampling;

                size_t iactable = findTable(des.acHuffmanTable, dht, true);
                size_t idctable = findTable(des.dcHuffmanTable, dht, false);


                ComponentDecoder ac(dht[iactable]);
                ComponentDecoder dc(dht[idctable]);


                for(int freqx = 0; freqx < samplingx; freqx++)
                {
                    int colDataUnit = colMCU * samplingx + freqx;

                    for(int freqy = 0; freqy < samplingy; freqy++)
                    {
                        int rowDataUnit = rowMCU * samplingy + freqy;

                        //auto dcCoef = dc.decodeDC(bitreader);
                        //unsigned char



                        //descomprimir coeficiente AC
                        //obtener tabla huffman que corresponde a el coeficiente aC
                        //de la seccion DHT sacar la lista con la cuenta de los tamaños de los codigos huffman
                        //tambien sacar la lista de valores huffman
                        //usar read stream para obtener el codigo ac


                    }
                }
            }

        }

    }


 }

 size_t reader::findTable(char idTable, vector<DHT> &tables, bool isdctable)
 {
     bool findDc = isdctable;

     char classNum = findDc ? 0 : 1;

    for(size_t i = tables.size() - 1; i >= 0; i--){
        auto table = tables[i];

        if(idTable == table.identifier && table.classNum == classNum){
            return i;
        }



    }

     throw std::logic_error("Could not find Huffman table");
 }


void reader::decodeCoeficient(SOS &scan, DHT &dht)
{

}

 void reader::getNumMCU(SOF0 &sos, unsigned char mxSamplingX, unsigned char mxSamplingY, unsigned int &outMCUx, unsigned int &outMCUy)
 {
     unsigned char numComponents = sos.components.size();
     uint2 width = sos.width;
     uint2 height = sos.height;


     if(numComponents > 1)//is jpeg interleaved
     {
         outMCUx = (width  + 8 * mxSamplingX - 1) / ( 8 * mxSamplingX);
         outMCUy = (height + 8 * mxSamplingY - 1) / (8 * mxSamplingY);
     }
     else
     {
         unsigned char cSamplingH = numComponents == 1 ? sos.components.at(0).horizontalSampling : 1;
         unsigned char cSamplingV = numComponents == 1 ? sos.components.at(0).verticalSampling : 1;

         unsigned int pixelsX = 8 * (mxSamplingX / cSamplingH);//si solo hay una componente max/c debe dar 1 y pixels es 8, para que usar otra formula del libro?
         unsigned int pixelsY = 8 * (mxSamplingY / cSamplingV);

         outMCUx = (width + pixelsX - 1) / pixelsX;
         outMCUy = (height + pixelsY - 1) / pixelsY;

     }

 }

 void reader::getMaxSampling(SOF0 &sos, unsigned char &horizontal, unsigned char &vertical)
 {
    horizontal = 0;
    vertical = 0;

    for(auto it = sos.components.begin(); it != sos.components.end(); ++it){
        auto e = ( *it);

        if(e.horizontalSampling > horizontal){
            horizontal = e.horizontalSampling;
        }

        if(e.verticalSampling > vertical){
            vertical = e.verticalSampling;
        }
    }
 }

void reader::readHeader()
{
    //std::ifstream stream(pathFile.c_str(), std::ios::binary);

    std::istream &stream = _stream;


    if(!stream.good())
        throw std::logic_error("could not open file");

    this->validateFile(stream);

    uint2 marker;

    read2bytes(stream, marker);

    if(marker == Markers::APP0){
        //TODO read extra app0 data
        std::logic_error("read extra app0 header is missing");
    }

    versionMajorId = (char)((marker & 0xFF00)>>8);
    versionMinorId = (char)(unsigned char)(marker & 0x00FF);


    stream.read(&units, 1);

    read2bytes(stream, xDensity);
    read2bytes(stream, yDensity);

    stream.read(&xThumbnail, 1);
    stream.read(&yThumbnail, 1);

    long int size = 3*((int)xThumbnail) * ((int)yThumbnail);

    if(size > 0){
        thumbnail.resize(size);
        stream.read(&thumbnail[0], size);
    }



}

void reader::readTable()
{
    uint2 twobytes = 0;

    read2bytes(_stream, twobytes);

    if(Markers::DHT == twobytes){
        std::cout<<"Read tables";
    }
}

void reader::read2bytes(istream &stream, uint2 &outbytes)
{
    unsigned char bytes[2];

    stream.read((char*)bytes, 2);

    outbytes = ((int)(bytes[0]) << 8 ) + bytes[1];

}

bool reader::moveToMark(uint2 searchMarker)
{
    bool found = false;
    unsigned char bytemarker[2];

    while(_stream && _stream.read((char*)bytemarker, 2) && !found){

        uint2 marker = ((int)(bytemarker[0]) << 8 ) + bytemarker[1];

        if(marker == searchMarker){
            found = true;
        }else{
            if(!Markers::isStandAlone(marker)){
                uint2 ln = 0;

                this->read2bytes(_stream, ln);
                _stream.seekg( ln - 2, std::ios_base::cur);

            }
        }




    }



    return found;

}






DHT::DHT()
{

}


}

