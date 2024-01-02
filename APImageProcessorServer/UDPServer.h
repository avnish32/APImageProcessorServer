#include<WinSock2.h>

#include<opencv2/opencv.hpp>
#include<map>
#include<chrono>

#include "ThreadPool.h"
#include "Constants.h"
#include "MsgLogger.h"

using std::map;
using std::string;
using std::vector;
using std::chrono::high_resolution_clock;

using cv::Mat;

#pragma once

//This server maintains a map to store messages received from each client.
//For each new client, a new queue is created and a new task is pushed to be picked up by the thread pool.
//The main thread is responsible only for receiving messages from clients and pushing them
//into the client's respective queue.
//The client task then consumes and processes the messages from the queue.

class UDPServer
{
private:
	SOCKET _socket;
	mutex _mtx;
	map<string, queue<string>> _clientToQueueMap;
	MsgLogger* _msgLogger = MsgLogger::GetInstance();

	void _ProcessImageReq(const sockaddr_in& clientAddress);

	//Utility functions
	const vector<string> _SplitString(char* inputString, char delimiter);
	const vector<string> _SplitString(char* inputString, const char& delimiter, const int& numberOfSplits, const int& inputStringLength);
	bool _HasRequestTimedOut(high_resolution_clock::time_point& lastImagePayloadRecdTime, const ushort& timeoutDuration);
	ushort _DrainQueue(std::queue<string>& clientQueue, string& imageSizeString);
	vector<u_short> _GetMissingPayloadSeqNumbers(const map<u_short, string>& receivedPayloadsMap, u_short expectedNumberOfPayloads);
	void _RemoveClientDataFromMap(const string& clientAddressKey);
	const string _MakeClientAddressKey(const sockaddr_in& clientAddress);
	short _InitializeImageMetadata(cv::Size& imageDimensions, uint& imageFileSize, ImageFilterTypesEnum& filterType, vector<float>& filterParams,
		std::queue<string>& clientQueue);
	short _ProcessImageMetadataPayload(char* receivedData, cv::Size& imageDimensions, uint& imageFileSize,
		ImageFilterTypesEnum& filterTypeEnum, vector<float>& filterParams);
	void _BuildImageDataPayloadMap(Mat image, map<u_short, string>& imageDataPayloadMap,
		map<u_short, u_short>& sequenceNumToPayloadSizeMap, vector<u_short>& sequenceNumbers);

	//Validation functions
	short _ValidateClientResponse(std::vector<string>& serverResponseSplit, short& serverResponseCode);
	short _ValidateImageDataPayload(std::vector<string>& splitImageDataPayload, u_int& payloadSeqNum, u_int& payloadSize);

	//Functions to send data to client
	short _SendServerResponseToClient(short serverResponseCode, const sockaddr_in& clientAddress, const vector<u_short>* missingSeqNumbers);
	short _SendMissingPayloadSeqNumbersToClient(std::map<u_short, string>& imagePayloadSeqMap, const u_short& expectedNumberOfPayloads,
		vector<u_short>& missingPayloadSeqNumbersInLastTimeout, const sockaddr_in& clientAddress);
	short _SendImageMetadataToClient(const Mat& image, const sockaddr_in& clientAddress);
	short _SendImage(const cv::Mat& imageToSend, const sockaddr_in& clientAddress, queue<string>& clientQueue);
	short _SendImageDataPayloadsBySequenceNumbers(map<u_short, string>& imageDataPayloadMap, map<u_short, u_short>& sequenceNumToPayloadSizeMap,
		const vector<u_short>& payloadSeqNumbersToSend, const sockaddr_in& serverAddress);

	//Functions to receive data from client
	short _ConsumeAndValidateClientMsgFromQueue(std::queue<string>& clientQueue, std::vector<cv::String>& clientResponseSplit, short& clientResponseCode);
	short _ConsumeImageDataFromClientQueue(std::queue<string>& clientQueue, std::map<u_short, std::string>& imagePayloadSeqMap,
		const u_short& expectedNumberOfPayloads, const sockaddr_in& clientAddress, long& imageBytesLeftToReceive);
	

	/*void processImageProcessingReq(char* receivedImageSizeData, const sockaddr_in clientAddress);
	const Mat constructImageFromData(const char* imageData, const cv::Size& imageDimensions);
	const Mat constructImageFromData(map<u_short, string> imageDataMap, const cv::Size& imageDimensions);*/

public:
	UDPServer();
	~UDPServer();
	
	short StartReceivingClientMsgs();
	bool IsValid();
	//short receiveImage(const cv::Size& imageDimensions, const sockaddr_in& clientAddress);
};

