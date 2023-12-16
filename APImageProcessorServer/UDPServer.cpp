#include "UDPServer.h"

#include<iostream>
#include<opencv2/opencv.hpp>

#pragma comment (lib, "ws2_32.lib")

using namespace std;
using namespace cv;

UDPServer::UDPServer()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) == -1) {
		cout << "\nError while initialising WSA.";
		_socket = INVALID_SOCKET;
	}
	else {
		_socket = socket(AF_INET, SOCK_DGRAM, 0);
		if (_socket == INVALID_SOCKET) {
			cout << "\nError while creating socket. Error code: " << WSAGetLastError();
		}
		else {
			cout << "\nSocket created successfully.";
			sockaddr_in serverAddress;
			serverAddress.sin_family = AF_INET;
			serverAddress.sin_addr.s_addr = ADDR_ANY;
			serverAddress.sin_port = 8080;
			
			if (bind(_socket, (const sockaddr*)&serverAddress, (int) sizeof(serverAddress)) == -1) {
				cout << "\nError while binding server socket. Error code: " << WSAGetLastError();
				_socket = INVALID_SOCKET;
			}
			else {
				cout << "\nSocked bound successfully.";
			}
		}
	}
}

UDPServer::~UDPServer()
{
	WSACleanup();
	closesocket(_socket);
}

int UDPServer::receiveImageSize()
{
	imageDimensions = cv::Size(1024, 768); //TODO receive this from client, preferably as a cv::size object
	return 1; //TODO return number of bytes recd here
}

int UDPServer::receiveImage()
{
	if (_socket == INVALID_SOCKET) {
		cout << "\nERROR: Invalid socket.";
		return -1;
	}

	long imageSize = imageDimensions.width * imageDimensions.height * 3;
	char* recdImageData = new char[imageSize];
	int clientAddrSize = sizeof(clientAddress);
	long bytesRecd = 0, bytesLeftToReceive = imageSize;
	cout << "\nImage size before receiving: " << imageSize;
	while (bytesRecd < imageSize) {
		int bytesRecdThisIteration;

		cout << "\nBytes left to receive: " << bytesLeftToReceive;

		if (bytesLeftToReceive >= 60000l) {
			bytesRecdThisIteration = recvfrom(_socket, recdImageData + bytesRecd, 60000l, 0, (sockaddr*)&clientAddress, &clientAddrSize);
		}
		else {
			bytesRecdThisIteration = recvfrom(_socket, recdImageData + bytesRecd, bytesLeftToReceive, 0, (sockaddr*)&clientAddress, &clientAddrSize);
		}
		
		if (bytesRecdThisIteration <= 0) {
			cout << "\nError in receiving image data. Error code: "<<WSAGetLastError();
			return bytesRecd;
		}
		bytesRecd += bytesRecdThisIteration;
		bytesLeftToReceive -= bytesRecdThisIteration;
		
		cout << "\nBytes recd this iteration: " << bytesRecdThisIteration;
	}

	//TODO move all image processing out of here and check hash
	cout << "\nAll data recd. Re-shaping image now...";
	Mat recdImage = Mat(imageDimensions, CV_8UC3);
	//long recdImgData;
	for (int i = 0; i < imageDimensions.height; i++) {
		for (int j = 0; j < imageDimensions.width; j++) {
			recdImage.at<Vec3b>(i, j) = Vec3b(*(recdImageData), *(recdImageData + 1), *(recdImageData + 2));
			recdImageData += 3;
		}
	}

	cout << "\nImage re-shaped.";
	cv::String windowName = "Received Image";
	namedWindow(windowName, WINDOW_NORMAL);
	imshow(windowName, recdImage);

	waitKey(0);
	destroyWindow(windowName);

	return bytesRecd;

}
