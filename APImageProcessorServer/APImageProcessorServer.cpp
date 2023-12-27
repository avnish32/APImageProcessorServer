// APImageProcessorServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "UDPServer.h"
#include "ThreadPool.h"
#include "Constants.h"

#include<fstream>


int main()
{
    //Below snippet to redirect cout buffer to external file was taken from https://gist.github.com/mandyedi/ae68a3191096222c62655d54935e7bb2
    //Performs 9 times faster when output is written to file.
    std::ofstream out("outLogs.txt");
    std::streambuf* coutbuf = std::cout.rdbuf(); //save old buf
    std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

    std::cout << "Hello World!\n";

    UDPServer udpServer;
    if (!udpServer.isValid()) {
        cout << "\nError in socket creation. Application will exit now.";
        return RESPONSE_FAILURE;
    }

    //ThreadPool threadPool(NUM_THREADS);
    
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
    
    std::cout.rdbuf(coutbuf); //reset to standard output again
}
