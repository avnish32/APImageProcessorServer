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
	SOCKET socket_;
	mutex mtx_;
	map<string, queue<string>> client_queue_map_;
	MsgLogger* msg_logger_ = MsgLogger::GetInstance();

	void ProcessImageReq(const sockaddr_in& clientAddress);

	//Utility functions
	const vector<string> SplitString(char* inputString, char delimiter);
	const vector<string> SplitString(char* inputString, const char& delimiter, const int& numberOfSplits, const int& inputStringLength);
	bool HasRequestTimedOut(high_resolution_clock::time_point& lastImagePayloadRecdTime, const u_short& timeoutDuration);
	u_short DrainQueue(std::queue<string>& clientQueue, string& imageSizeString);
	vector<u_short> GetMissingPayloadSeqNumbers(const map<u_short, string>& receivedPayloadsMap, u_short expectedNumberOfPayloads);
	void RemoveClientDataFromMap(const string& clientAddressKey);
	const string MakeClientAddressKey(const sockaddr_in& clientAddress);
	short InitializeImageMetadata(cv::Size& imageDimensions, uint& imageFileSize, ImageFilterTypesEnum& filterType, vector<float>& filterParams,
		std::queue<string>& clientQueue);
	short ProcessImageMetadataPayload(char* receivedData, cv::Size& imageDimensions, uint& imageFileSize,
		ImageFilterTypesEnum& filterTypeEnum, vector<float>& filterParams);
	void BuildImageDataPayloadMap(Mat image, map<u_short, string>& imageDataPayloadMap,
		map<u_short, u_short>& sequenceNumToPayloadSizeMap, vector<u_short>& sequenceNumbers);

	//Validation functions
	short ValidateClientResponse(std::vector<string>& serverResponseSplit, short& serverResponseCode);
	short ValidateImageDataPayload(std::vector<string>& splitImageDataPayload, u_int& payloadSeqNum, u_int& payloadSize);

	//Functions to send data to client
	short SendServerResponseToClient(short serverResponseCode, const sockaddr_in& clientAddress, const vector<u_short>* missingSeqNumbers);
	short SendMissingPayloadSeqNumbersToClient(std::map<u_short, string>& imagePayloadSeqMap, const u_short& expectedNumberOfPayloads,
		vector<u_short>& missingPayloadSeqNumbersInLastTimeout, const sockaddr_in& clientAddress);
	short SendImageMetadataToClient(const Mat& image, const sockaddr_in& clientAddress);
	short SendImage(const cv::Mat& imageToSend, const sockaddr_in& clientAddress, queue<string>& clientQueue);
	short SendImageDataPayloadsBySequenceNumbers(map<u_short, string>& imageDataPayloadMap, map<u_short, u_short>& sequenceNumToPayloadSizeMap,
		const vector<u_short>& payloadSeqNumbersToSend, const sockaddr_in& serverAddress);

	//Functions to receive data from client
	short ConsumeAndValidateClientMsgFromQueue(std::queue<string>& clientQueue, std::vector<cv::String>& clientResponseSplit, short& clientResponseCode);
	short ConsumeImageDataFromClientQueue(std::queue<string>& clientQueue, std::map<u_short, std::string>& imagePayloadSeqMap,
		const u_short& expectedNumberOfPayloads, const sockaddr_in& clientAddress, long& imageBytesLeftToReceive);

public:
	UDPServer();
	~UDPServer();
	
	short StartReceivingClientMsgs();
	bool IsValid();
};

