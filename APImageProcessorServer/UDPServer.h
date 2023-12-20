#include<WinSock2.h>

#include<opencv2/opencv.hpp>
#include "ThreadPool.h"

using namespace cv;

#pragma once
class UDPServer
{
private:
	SOCKET _socket;
	mutex _mtx;

	//TODO map to store image size for each client - we might have to spawn new thread for each client, so each thread
	//can have its own copy of the imageDimensions sent by the client.
	//Also since server does not know the client details before it receives data in UDP, there's no way
	//to check the map if it contains data for the connecting client before actually receiving the image from client,
	//which is a deadlock.

	void processImageProcessingReq(char* receivedImageSizeData, const sockaddr_in clientAddress);
	const Mat constructImageFromData(char* recdImageData, const cv::Size& imageDimensions);
	short processImageSizePayload(char* receivedData, cv::Size& imageDimensions);

public:
	UDPServer();
	~UDPServer();
	short sendAck(short serverResponseCode, const sockaddr_in& clientAddress);
	short receiveImageSize();
	short receiveImage(const cv::Size& imageDimensions, const sockaddr_in& clientAddress); //TODO use abstract class here: Server -> UDPServer -> ImageReceivingServer
	bool isValid();
};

