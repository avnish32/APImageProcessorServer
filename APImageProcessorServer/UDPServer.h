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

	void ProcessImageReq(const sockaddr_in&);

	//Utility functions
	const vector<string> SplitString(char*, char);
	const vector<string> SplitString(char*, const char&, const int&, const int&);
	bool HasRequestTimedOut(high_resolution_clock::time_point&, const u_short&);
	void ClearQueue(std::queue<string>&);
	u_short DrainQueue(std::queue<string>&, string&);
	vector<u_short> GetMissingPayloadSeqNumbers(const map<u_short, string>&, u_short);
	void RemoveClientDataFromMap(const string&);
	const string MakeClientAddressKey(const sockaddr_in&);
	short InitializeImageMetadata(cv::Size&, uint&, ImageFilterTypesEnum&, vector<float>&,
		std::queue<string>&);
	short ProcessImageMetadataPayload(char*, cv::Size&, uint&,
		ImageFilterTypesEnum&, vector<float>&);
	void BuildImageDataPayloadMap(Mat, map<u_short, string>&,
		map<u_short, u_short>&, vector<u_short>&);

	//Validation functions
	short ValidateClientResponse(std::vector<string>&, short&);
	short ValidateImageDataPayload(std::vector<string>&, u_int&, u_int&);

	//Functions to send data to client
	short SendServerResponseToClient(short, const sockaddr_in&, const vector<u_short>*);
	short SendMissingPayloadSeqNumbersToClient(std::map<u_short, string>&, const u_short&,
		vector<u_short>&, const sockaddr_in&);
	short SendImageMetadataToClient(const Mat&, const sockaddr_in&);
	short SendImage(const cv::Mat&, const sockaddr_in&, queue<string>&);
	short SendImageDataPayloadsBySequenceNumbers(map<u_short, string>&, map<u_short, u_short>&,
		const vector<u_short>&, const sockaddr_in&);

	//Functions to receive data from client
	short ConsumeAndValidateClientMsgFromQueue(std::queue<string>&, std::vector<cv::String>&, short&);
	short ConsumeImageDataFromClientQueue(std::queue<string>&, std::map<u_short, std::string>&,
		const u_short&, const sockaddr_in&, long&);

public:
	UDPServer();
	~UDPServer();
	
	short StartReceivingClientMsgs();
	bool IsValid();
};

