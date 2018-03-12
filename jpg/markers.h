#ifndef MARKERS_H
#define MARKERS_H

namespace jpg{
typedef  unsigned short uint2;

class Markers
{
public:
    Markers();
    const static uint2 SOI = 0xFFD8;//start of image
       const static uint2 EOI = 0xFFD9;//END OF IMAGE
       const static uint2 APP0 = 0xFFE0;//Application marker
       const static uint2 DHT = 0xFFC4;//define huffman table
       const static uint2 DQT = 0xFFDB; //define quantization table
       const static uint2 SOF0 = 0xFFC0; //start of frame, baseline
       const static uint2 SOS = 0xFFDA;
       const static uint2 DRI = 0xFFDD; //define restart interval
       const static uint2 TEM = 0xFF01;

       static bool isAPP(uint2 marker);
       static bool isRST(uint2 marker);

       static bool isMarker(uint2 marker);
       static bool isStandAlone(uint2 marker);
};

}



#endif // MARKERS_H
