#include<WinSock2.h>

#include<opencv2/opencv.hpp>

#pragma once
class UDPServer
{
private:
	SOCKET _socket;
	sockaddr_in clientAddress;
	cv::Size imageDimensions; //TODO map to store image size for each client

public:
	UDPServer();
	~UDPServer();
	int receiveImageSize();
	int receiveImage(); //TODO use abstract class here: Server -> UDPServer -> ImageReceivingServer
	//TODO bool isSocketValid();
};

