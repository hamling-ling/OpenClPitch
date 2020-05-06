#include <iostream>
#include "../PitchDetector/PitchDetector/PitchDetector.h"

using namespace std;

int main(int argc, char *argv[])
{
    PitchDetector detector(44.1*1000, 1024);
    
    cout << "Hello World" << endl;
    return 0;
}
