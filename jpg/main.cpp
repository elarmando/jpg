#include <iostream>

using namespace std;
#include "bitreader.h"
#include "componentdecoder.h"
#include "reader.h"

void test_bitreader_readbits__shouldReadOk(){
    std::vector<char> v;

    v.push_back((unsigned char)128);

    jpg::BitReader reader(&v);

    unsigned char res = reader.readBits((unsigned char)3);

    if(res == 4)
        cout<<"success"<<std::endl;
    else
        cout<<"Error"<<std::endl;


}

void test_componentdecoder_extend__shouldReadOk(){
    jpg::DHT dth;

    jpg::ComponentDecoder c(dth);

    unsigned char additional = 0 + 64;
    unsigned char magnitude = 8;

    int a = c.extend(additional, magnitude);

    if(a == -64)
        cout<<"success"<<std::endl;
    else
        cout<<"Error"<<std::endl;


}


void tests(){
    //test_bitreader_readbits__shouldReadOk();
    test_componentdecoder_extend__shouldReadOk();
}

int main()
{
    jpg::reader reader("test.jpg");
    reader.read();

    //tests();

    char e = -56;
    unsigned char e2 = e;

    std::cout<<e2;

    return 0;
}

