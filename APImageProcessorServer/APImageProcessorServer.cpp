// APImageProcessorServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "UDPServer.h"
#include "Constants.h"

int main()
{
    std::cout << "Hello World!\n";

    UDPServer udpServer;
    if (!udpServer.isValid()) {
        cout << "\nError in socket creation. Application will exit now.";
        return RESPONSE_FAILURE;
    }
    
    short responseCode = udpServer.receiveImageSize();
    short serverResponseCode = SERVER_POSITIVE_ACK;
    if (responseCode == RESPONSE_FAILURE) {
        serverResponseCode = SERVER_NEGATIVE_ACK;
    }
    responseCode = udpServer.sendAck(serverResponseCode);
    if (responseCode == RESPONSE_FAILURE) {
        cout << "\nError in sending acknowldgement. Application will exit now.";
        return RESPONSE_FAILURE;
    }
    if (serverResponseCode == SERVER_POSITIVE_ACK) {
        udpServer.receiveImage();
    }
    
}
