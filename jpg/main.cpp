#include <iostream>

using namespace std;
#include "reader.h"

int main()
{
    jpg::reader reader("test.jpg");
    reader.read();

    cout << "Hello World!" << endl;
    return 0;
}

