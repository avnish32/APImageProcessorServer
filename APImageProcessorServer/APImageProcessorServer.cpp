// APImageProcessorServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "UDPServer.h"
#include "ThreadPool.h"
#include "Constants.h"
#include "MsgLogger.h"

#include<fstream>

MsgLogger* MsgLogger::_loggerInstance = nullptr;

int main()
{
    MsgLogger* msgLogger = MsgLogger::GetInstance();

    cout << "Server application started.";

    UDPServer udpServer;
    if (!udpServer.isValid()) {
        //cout << "\nError in socket creation. Application will exit now.";
        msgLogger->LogError("Error in socket creation. Application will exit now.");
        return RESPONSE_FAILURE;
    }
    
    short responseCode = udpServer.ReceiveClientMsg();
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
