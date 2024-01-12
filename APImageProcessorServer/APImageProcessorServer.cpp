// APImageProcessorServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "UDPServer.h"
#include "ThreadPool.h"
#include "Constants.h"
#include "MsgLogger.h"

#include<fstream>

using std::cout;

MsgLogger* MsgLogger::logger_instance_ = nullptr;

int main()
{
    MsgLogger* msg_logger = MsgLogger::GetInstance();

    cout << "Server application started.";

    UDPServer udp_server;
    if (!udp_server.IsValid()) {
        msg_logger->LogError("Error in socket creation. Application will exit now.");
        return RESPONSE_FAILURE;
    }
    
    udp_server.StartReceivingClientMsgs();
}
