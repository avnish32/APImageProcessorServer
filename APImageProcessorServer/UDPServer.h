#include<WinSock2.h>

#include<opencv2/opencv.hpp>
#include<map>

#include "ThreadPool.h"
#include "Constants.h"
#include "MsgLogger.h"

//TODO subtitute this
using namespace cv;

#pragma once
class UDPServer
{
private:
	SOCKET _socket;
	mutex _mtx;
	map<std::string, queue<std::string>> _clientToQueueMap;
	MsgLogger* _msgLogger = MsgLogger::GetInstance();	

	const vector<std::string> SplitString(char* inputString, char delimiter);
	const vector<string> SplitString(char* inputString, const char& delimiter, const int& numberOfSplits, const int& inputStringLength);
	short ValidateClientResponse(std::vector<std::string>& serverResponseSplit, short& serverResponseCode);
	void ProcessImageReq(const sockaddr_in& clientAddress);
	short ConsumeImageDataFromClientQueue(std::queue<std::string>& clientQueue, std::map<u_short, std::string>& imagePayloadSeqMap,
		const u_short& expectedNumberOfPayloads, const sockaddr_in& clientAddress, long& imageBytesLeftToReceive);
	short ValidateImageDataPayload(std::vector<std::string>& splitImageDataPayload, u_int& payloadSeqNum, u_int& payloadSize);
	short CheckForTimeout(std::chrono::steady_clock::time_point& lastImagePayloadRecdTime,
		std::map<u_short, std::string>& imagePayloadSeqMap, const u_short& expectedNumberOfPayloads,
		const sockaddr_in& clientAddress);
	short InitializeImageMetadata(cv::Size& imageDimensions, uint& imageFileSize, ImageFilterTypesEnum& filterType, vector<float>& filterParams,
		std::queue<string>& clientQueue);
	long DrainQueue(std::queue<std::string>& clientQueue, std::string& imageSizeString);
	void processImageProcessingReq(char* receivedImageSizeData, const sockaddr_in clientAddress);
	const Mat constructImageFromData(const char* imageData, const cv::Size& imageDimensions);
	const Mat constructImageFromData(map<u_short, string> imageDataMap, const cv::Size& imageDimensions);
	short ProcessImageMetadataPayload(char* receivedData, cv::Size& imageDimensions, uint& imageFileSize, 
		ImageFilterTypesEnum& filterTypeEnum, vector<float>& filterParams);
	vector<u_short> calculateMissingPayloadSeqNumbers(const map<u_short, string>& receivedPayloadsMap, u_short expectedNumberOfPayloads);
	void BuildImageDataPayloadMap(Mat image, map<u_short, string>& imageDataPayloadMap,
		map<u_short, u_short>& sequenceNumToPayloadSizeMap, vector<u_short>& sequenceNumbers);
	short SendImageDataPayloadsBySequenceNumbers(map<u_short, string>& imageDataPayloadMap, map<u_short, u_short>& sequenceNumToPayloadSizeMap,
		const vector<u_short>& payloadSeqNumbersToSend, const sockaddr_in& serverAddress);
	void _RemoveClientDataFromMap(const string& clientAddressKey);

public:
	UDPServer();
	~UDPServer();
	short SendServerResponseToClient(short serverResponseCode, const sockaddr_in& clientAddress, const vector<u_short>* missingSeqNumbers);
	short SendImageMetadataToClient(const Mat& image, const sockaddr_in& clientAddress);
	short ReceiveClientMsg();
	short receiveImage(const cv::Size& imageDimensions, const sockaddr_in& clientAddress); //TODO use abstract class here: Server -> UDPServer -> ImageReceivingServer
	short SendImage(const cv::Mat& imageToSend, const sockaddr_in& clientAddress, queue<std::string>& clientQueue);
	short ConsumeAndValidateClientMsgFromQueue(std::queue<std::string>& clientQueue, std::vector<cv::String>& clientResponseSplit, short& clientResponseCode);
	bool isValid();
};

