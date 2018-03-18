#include <iostream>

using namespace std;
#include "reader.h"

int main()
{
    jpg::reader reader("test.jpg");
    reader.read();


    return 0;
}

