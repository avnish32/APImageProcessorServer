// APImageProcessorServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "UDPServer.h"

using namespace std;

int main()
{
    std::cout << "Hello World!\n";

    UDPServer udpServer;
    udpServer.receiveImageSize();
    udpServer.receiveImage();
}
