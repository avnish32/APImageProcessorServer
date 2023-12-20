// APImageProcessorServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "UDPServer.h"
#include "ThreadPool.h"
#include "Constants.h"


int main()
{
    std::cout << "Hello World!\n";

    UDPServer udpServer;
    if (!udpServer.isValid()) {
        cout << "\nError in socket creation. Application will exit now.";
        return RESPONSE_FAILURE;
    }

    //ThreadPool threadPool(NUM_THREADS);
    
    short responseCode = udpServer.receiveImageSize();
    /*if (responseCode == RESPONSE_FAILURE) {
        responseCode = udpServer.sendAck(SERVER_NEGATIVE_ACK);
        if (responseCode == RESPONSE_FAILURE) {
            cout << "\nError in sending acknowldgement. Client disconnected.";
        }
    }*/
    
    /*if (serverResponseCode == SERVER_POSITIVE_ACK) {
        udpServer.receiveImage();
    }*/
    
}
