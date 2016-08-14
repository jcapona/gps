#include <gps.h>
#include <iostream>
#include <string>

int main()
{
    gps dev("/dev/ttyUSB0",4800);

    while (1) {
        dev.read();
        std::cout << dev.getTime() <<
            " Lat: " <<dev.getLatitude() <<
            " Long: " <<dev.getLongitude() <<
            " Alt: " <<dev.getAltitude() <<
            " Speed: " <<dev.getSpeed() <<"\n";
    }
    return 0;
}
