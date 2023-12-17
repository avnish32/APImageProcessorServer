#include<WinSock2.h>

#include<opencv2/opencv.hpp>

using namespace cv;

#pragma once
class UDPServer
{
private:
	SOCKET _socket;
	sockaddr_in _clientAddress;
	cv::Size _imageDimensions; 
	//TODO map to store image size for each client - we might have to spawn new thread for each client, so each thread
	//can have its own copy of the imageDimensions sent by the client.
	//Also since server does not know the client details before it receives data in UDP, there's no way
	//to check the map if it contains data for the connecting client before actually receiving the image from client,
	//which is a deadlock.

	const Mat constructImageFromData(char* recdImageData);

public:
	UDPServer();
	~UDPServer();
	short sendAck(short serverResponseCode);
	short receiveImageSize();
	short receiveImage(); //TODO use abstract class here: Server -> UDPServer -> ImageReceivingServer
	bool isValid();
};

