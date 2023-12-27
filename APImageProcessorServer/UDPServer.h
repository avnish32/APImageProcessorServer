#include<WinSock2.h>

#include<opencv2/opencv.hpp>
#include<map>

#include "ThreadPool.h"
#include "Constants.h"

using namespace cv;

#pragma once
class UDPServer
{
private:
	SOCKET _socket;
	mutex _mtx;
	map<std::string, queue<std::string>> _clientToQueueMap;

	//TODO map to store image size for each client - we might have to spawn new thread for each client, so each thread
	//can have its own copy of the imageDimensions sent by the client.
	//Also since server does not know the client details before it receives data in UDP, there's no way
	//to check the map if it contains data for the connecting client before actually receiving the image from client,
	//which is a deadlock.

	const vector<std::string> SplitString(char* inputString, char delimiter);
	const vector<string> SplitString(char* inputString, const char& delimiter, const int& numberOfSplits, const int& inputStringLength);
	short validateClientResponse(std::vector<std::string>& serverResponseSplit, short& serverResponseCode);
	void processImageReq(const sockaddr_in& clientAddress);
	short CheckForTimeout(std::chrono::steady_clock::time_point& lastImagePayloadRecdTime,
		std::map<u_short, std::string>& imagePayloadSeqMap, const u_short& expectedNumberOfPayloads,
		const sockaddr_in& clientAddress);
	short InitializeImageMetadata(cv::Size& imageDimensions, ImageFilterTypesEnum& filterType, vector<float>& filterParams,
		std::queue<string>& clientQueue);
	long DrainQueue(std::queue<std::string>& clientQueue, std::string& imageSizeString);
	void processImageProcessingReq(char* receivedImageSizeData, const sockaddr_in clientAddress);
	const Mat constructImageFromData(const char* imageData, const cv::Size& imageDimensions);
	const Mat constructImageFromData(map<u_short, string> imageDataMap, const cv::Size& imageDimensions);
	short processImageMetadataPayload(char* receivedData, cv::Size& imageDimensions, ImageFilterTypesEnum& filterTypeEnum, vector<float>& filterParams);
	vector<u_short> calculateMissingPayloadSeqNumbers(const map<u_short, string>& receivedPayloadsMap, u_short expectedNumberOfPayloads);
	void buildImageDataPayloadMap(Mat image, map<u_short, string>& imageDataPayloadMap,
		map<u_short, u_short>& sequenceNumToPayloadSizeMap, vector<u_short>& sequenceNumbers);
	short sendImageDataPayloadsBySequenceNumbers(map<u_short, string>& imageDataPayloadMap, map<u_short, u_short>& sequenceNumToPayloadSizeMap,
		const vector<u_short>& payloadSeqNumbersToSend, const sockaddr_in& serverAddress);

public:
	UDPServer();
	~UDPServer();
	short sendServerResponse(short serverResponseCode, const sockaddr_in& clientAddress, const vector<u_short>* missingSeqNumbers);
	short ReceiveClientMsg();
	short receiveImage(const cv::Size& imageDimensions, const sockaddr_in& clientAddress); //TODO use abstract class here: Server -> UDPServer -> ImageReceivingServer
	short SendImage(const cv::Mat& imageToSend, const sockaddr_in& clientAddress, queue<std::string>& clientQueue);
	bool isValid();
};

