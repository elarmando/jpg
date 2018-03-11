#include "markers.h"

namespace jpg{

Markers::Markers()
{

}

bool Markers::isAPP(uint2 marker)
{
    return 0xFFE0 <= marker && marker <= 0xFFEF;
}

bool Markers::isRST(uint2 marker)
{
    return marker >= 0xFFD0 && marker <= 0xFFD7;
}

bool Markers::isMarker(uint2 marker)
{
    return (marker & 0xFF00) == 0xFF00 && (marker & 0x00FF) != 0x0000;

}

}



