#include "UDPServer.h"
#include "Constants.h"
#include "ImageProcessor.h"

//TODO check if needed in final code
#include<format>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

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
			u_long NON_BLOCKING_MODE_TRUE = 0;
			if (ioctlsocket(_socket, FIONBIO, &NON_BLOCKING_MODE_TRUE) == -1) {
				cout << "\nCould not set server socket to non-blocking mode.";
				_socket = INVALID_SOCKET;
			}
			sockaddr_in serverAddress;
			serverAddress.sin_family = AF_INET;
			serverAddress.sin_addr.s_addr = ADDR_ANY;
			serverAddress.sin_port = 8080;
			
			if (bind(_socket, (const sockaddr*)&serverAddress, (int) sizeof(serverAddress)) == -1) {
				cout << "\nError while binding server socket. Error code: " << WSAGetLastError();
				_socket = INVALID_SOCKET;
			}
			else {
				cout << "\nSocket bound successfully.";
			}
		}
	}
	//_threadPool(NUM_THREADS);
}

UDPServer::~UDPServer()
{
	cout << "\nUDPServer destructor called.";
	WSACleanup();
	closesocket(_socket);
}

//---------------Utility functions
cv::String GetAddressToSaveImage() {

	//Below snippet to convert thread id to string taken from https://stackoverflow.com/a/19255203
	auto threadId = this_thread::get_id();
	stringstream sStream;
	sStream << threadId;

	//Below snippet to convert chrono::time_point to string taken from https://stackoverflow.com/a/46240575
	//using namespace std::chrono_literals;
	std::chrono::time_point tp = chrono::system_clock::now();
	std::string timestamp = std::format("{:%H%M%S}", tp);

	//string timestamp = std::format("{:%H%M%s}", nowTime);
	cv::String imageSaveAddress = "./Resources/savedImage_" + sStream.str() + "_" + timestamp + ".jpg";
	return imageSaveAddress;
}

const vector<std::string> splitString(char* inputString, char delimiter) {
	std::string currentWord = "";
	vector<std::string> outputVector;
	char* charPtr = inputString;
	//cout << "\nString array at beginning: " << *stringArray;


	while (*(charPtr) != '\0') {
		char currentChar = *(charPtr);
		//cout << "\nCurrent word: " << currentWord<<" | current char = "<<currentChar;
		if (currentChar == delimiter) {
			outputVector.push_back(currentWord);
			currentWord = "";
		}
		else {
			currentWord += currentChar;
		}
		++charPtr;
	}
	if (currentWord.length() > 0) {
		outputVector.push_back(currentWord);
	}

	//cout << "\nString after splitting: ";
	/*auto iter = outputVector.begin();
	while (iter != outputVector.end()) {
		cout << *iter << "|";
		iter++;
	}*/
	//cout << "\nReturning output vector. ";
	return outputVector;
}

const vector<string> splitString(char* inputString, const char& delimiter, const int& numberOfSplits, const int& inputStringLength) {
	std::string currentWord = "";
	int numberOfCurrentSplits = 1;
	vector<string> outputVector;
	//char* charPtr = inputString;
	//cout << "\nString array at beginning: " << *stringArray;

	for (int i = 0; i < inputStringLength; i++) {
		char currentChar = *(inputString + i);
		//cout << "\nCurrent word: " << currentWord<<" | current char = "<<currentChar;
		if (currentChar == delimiter && numberOfCurrentSplits < numberOfSplits) {
			outputVector.push_back(currentWord);
			currentWord = "";
			numberOfCurrentSplits++;
		}
		else {
			currentWord += currentChar;
		}
		//++charPtr;
	}

	if (currentWord.length() > 0) {
		outputVector.push_back(currentWord);
	}

	//cout << "\nString after splitting: ";
	/*auto iter = outputVector.begin();
	while (iter != outputVector.end()) {
		cout << *iter << "|";
		iter++;
	}*/
	//cout << "\nReturning output vector. ";
	if (outputVector.size() < numberOfSplits) {
		cout << "\nUnexpected split. Original string: " << inputString;
	}
	return outputVector;
}

short checkImageDataMapIntegrity(map<u_short, string>* imageDataMap, const u_int& expectedNumberOfPayloads) {
	cout << "\nChecking integrity of image data map.";

	for (u_int i = 1; i <= expectedNumberOfPayloads; i++) {
		if (imageDataMap->count(i) == 0) {
			cout << "\nERROR::Data not present in map for payload sequence " << i;
			return RESPONSE_FAILURE;
		}
	}

	cout << "\nImage data map integrity checked.";
	return RESPONSE_SUCCESS;
}

//------------Member functions

short UDPServer::receiveImageSize()
{
	if (_socket == INVALID_SOCKET) {
		cout << "\nERROR: Invalid socket.";
		return RESPONSE_FAILURE;
	}

	//Created local thread pool so threads will be joined before UDP Server gets destroyed.
	//TODO This invalidates the overloaded operator() of thread pool.
	ThreadPool threadPool(NUM_THREADS);
	//_threadPool(NUM_THREADS);

	while (true) {
		char* receivedData = new char[60025];
		sockaddr_in clientAddress;
		int clientAddrSize = sizeof(clientAddress);

		//_mtx.lock();
		//cout << "\nWaiting for image size...";
		long bytesRecdThisIteration = recvfrom(_socket, receivedData, 60025L, 0, (sockaddr*)&clientAddress, &clientAddrSize);
		//_mtx.unlock();

		if (bytesRecdThisIteration == SOCKET_ERROR) {
			int lastError = WSAGetLastError();
			//cout << "\nError in receiving image size data. Error code: " << lastError;

			if (lastError != WSAEWOULDBLOCK) {
				cout << "\nError in receiving image size data. Error code: " << lastError;
				sendServerResponse(SERVER_NEGATIVE_ACK, clientAddress, nullptr);
				return RESPONSE_FAILURE;
			}

			//cout << "\nNo data received in this iteration. Sleeping and retrying...";
			//this_thread::sleep_for(chrono::milliseconds(50));

		}
		else {
			cout << "\nImage data recd from client.";
			string clientAddressKey = to_string(clientAddress.sin_addr.s_addr) + ":;" + to_string(clientAddress.sin_port);
			_mtx.lock();
			if (_clientToQueueMap.count(clientAddressKey) <= 0) {
				cout << "\nMap entry not found for client address key: " << clientAddressKey;
				queue<string> newClientQueue;
				_clientToQueueMap[clientAddressKey] = newClientQueue;
				threadPool.enqueue(bind(&UDPServer::processImageReq, this, clientAddress));
			}
			
			//_clientToQueueMap[clientAddressKey].push(to_string(bytesRecdThisIteration));
			string imageData = string(receivedData, bytesRecdThisIteration);
			_clientToQueueMap[clientAddressKey].push(imageData);
			cout << "\nImage data size: " << imageData.length();
			cout << "\nImage data pushed to client queue. bytesRecdThisIterationString: "<< to_string(bytesRecdThisIteration);
			_mtx.unlock();
		}

		delete[] receivedData;
	}

	//TODO infinite loop to keep listening to clients, and possibly a timeout to exit the loop
	//while (true) {
	//	short serverResponseCodeForClient = SERVER_POSITIVE_ACK;
	//	char* receivedData = new char[15]; //Size 9999 9999\0
	//	sockaddr_in clientAddress;
	//	int clientAddrSize = sizeof(clientAddress);
	//	short bytesRecd = 0;
	//	while (bytesRecd < 15) {
	//		_mtx.lock();
	//		//cout << "\nWaiting for image size...";
	//		short bytesRecdThisIteration = recvfrom(_socket, receivedData + bytesRecd, 15, 0, (sockaddr*)&clientAddress, &clientAddrSize);
	//		_mtx.unlock();

	//		if (bytesRecdThisIteration == SOCKET_ERROR) {
	//			int lastError = WSAGetLastError();
	//			//cout << "\nError in receiving image size data. Error code: " << lastError;

	//			if (lastError != WSAEWOULDBLOCK) {
	//				cout << "\nError in receiving image size data. Error code: " << lastError;
	//				sendAck(SERVER_NEGATIVE_ACK, clientAddress);
	//				return RESPONSE_FAILURE;
	//			}

	//			//cout << "\nNo data received in this iteration. Sleeping and retrying...";
	//			this_thread::sleep_for(chrono::milliseconds(50));
	//			
	//		}
	//		else {
	//			bytesRecd += bytesRecdThisIteration;
	//		}
	//	}

	//	//TODO store future here and decide where to call future.get()
	//	threadPool.enqueue(bind(&UDPServer::processImageProcessingReq, this, receivedData, clientAddress));
	//	//return processImageSizePayload(receivedData);
	//}
}

void UDPServer::processImageReq(const sockaddr_in& clientAddress)
{
	cout << "\nThread: " << this_thread::get_id()<<" inside processImageReq.";
	string clientAddressKey = to_string(clientAddress.sin_addr.s_addr) + ":;" + to_string(clientAddress.sin_port);
	_mtx.lock();
	queue<string>& clientQueue = _clientToQueueMap[clientAddressKey];
	_mtx.unlock();

	cout << "\nClient queue size: " << clientQueue.size();

	cv::Size imageDimensions;
	short responseCode;

	short serverResponseCodeForClient = SERVER_POSITIVE_ACK;

	//TODO perform payload validations here, including checking whether filter can be applied
	responseCode = InitializeImageDimensions(imageDimensions, clientQueue);

	if (responseCode == RESPONSE_FAILURE) {
		serverResponseCodeForClient = SERVER_NEGATIVE_ACK;
	}
	responseCode = sendServerResponse(serverResponseCodeForClient, clientAddress, nullptr);
	if (responseCode == RESPONSE_FAILURE) {
		cout << "\nCould not send acknowledgement to client.";
		return;
	}
	cout << "\nAck sent.";

	long imageBytesRecd = 0, imageBytesLeftToReceive = imageDimensions.width * imageDimensions.height * 3;
	
	u_short expectedNumberOfPayloads = imageBytesLeftToReceive / 60000;
	if (imageBytesLeftToReceive % 60000 > 0) {
		expectedNumberOfPayloads++;
	}

	//char* recdImageData = new char[imageBytesLeftToReceive];
	string imageDataString = "";
	map<u_short, string> imagePayloadSeqMap;

	auto lastImagePayloadRecdTime = chrono::high_resolution_clock::now();
	while (imageBytesLeftToReceive > 0) {
		
		if (clientQueue.empty()) {
			responseCode = CheckForTimeout(lastImagePayloadRecdTime, imagePayloadSeqMap, expectedNumberOfPayloads, clientAddress);
			if (responseCode == RESPONSE_FAILURE) {
				cout<<"\nError when sending response to client on timeout.";
				//TODO client cleanup
				return;
			}
			continue;
		}
		
		/*long lastPayloadSize = 0;
		try {
			lastPayloadSize = stol(clientQueue.front());
			clientQueue.pop();
		}
		catch (invalid_argument) {
			cout << "\nERROR::Invalid payload size value found in queue.";
		}

		cout << "\nLast payload size for image data: " << lastPayloadSize;*/

		if (clientQueue.empty()) {
			cout << "\nClient queue is empty before fetching image data.";
		}
		
		if (clientQueue.front().length() < 17) {
			cout << "\nUnexpected msg found in queue. Msg: " << clientQueue.front();
			clientQueue.pop();
		}
		cout << "\nQueue msg size before splitting: " << clientQueue.front().length();
		vector<string> splitImageDataPayload = splitString(&(clientQueue.front()[0]), ' ', 5, clientQueue.front().length());
		cout << "\nSplit image payload size: " << splitImageDataPayload.size();

		//TODO shift hardcoded values to constants
		if (splitImageDataPayload.size() != 5 || splitImageDataPayload.at(0) != SEQUENCE_PAYLOAD_KEY || splitImageDataPayload.at(2) != SIZE_PAYLOAD_KEY) {
			cout << "\nERROR: Image data payload in incorrect format. First word: "<<splitImageDataPayload.at(0);
			return;
		}

		u_int payloadSeqNum = 0, payloadSize = 0;
		try {
			payloadSeqNum = stoi(splitImageDataPayload.at(1));
			payloadSize = stoi(splitImageDataPayload.at(3));
		}
		catch (invalid_argument) {
			cout << "\nERROR: Image data payload sequence num or size not an int. Seq num: " << splitImageDataPayload.at(1) 
				<< " | Size:"<<splitImageDataPayload.at(3);
			return;
		}

		
		imagePayloadSeqMap[payloadSeqNum] = splitImageDataPayload.at(4);
		cout << "\nImage data after splitting: " << splitImageDataPayload.at(0) <<" | "<<splitImageDataPayload.at(1)<<" | "
			<<splitImageDataPayload.at(2)<<" | "<<splitImageDataPayload.at(3)<<" | Length of image data: "<<splitImageDataPayload.at(4).length();
		//imageDataString += splitImageDataPayload.at(4);
		clientQueue.pop();

		cout << "\nAfter popping image data payload from queue. Queue size: "<<clientQueue.size();

		imageBytesRecd += payloadSize;
		imageBytesLeftToReceive -= payloadSize;

		cout << "\nImage bytes recd: " << imageBytesRecd << " | image bytes left to receive: " << imageBytesLeftToReceive;
		lastImagePayloadRecdTime = chrono::high_resolution_clock::now();
	}

	//Check for data integrity - hash? - can match hash only after re-creating the image
	//Removed data integrity check as above loop is exited only when all bytes are received.
	
	cout << "\nAll image data received. Sending positive ACK to client.";
	responseCode = sendServerResponse(SERVER_POSITIVE_ACK, clientAddress, nullptr);
	if (responseCode == RESPONSE_FAILURE) {
		cout << "\nCould not send ACK to client.";
		return;
	}

	ImageProcessor imageProcessor(imagePayloadSeqMap, imageDimensions);
	imageProcessor.DisplayImage("Received Image");
	imageProcessor.SaveImage(GetAddressToSaveImage());
	

	//Removing entry from map once processing completes for the client
	_clientToQueueMap.erase(clientAddressKey);
}

short UDPServer::CheckForTimeout(std::chrono::steady_clock::time_point& lastImagePayloadRecdTime, 
	std::map<u_short, std::string>& imagePayloadSeqMap, const u_short& expectedNumberOfPayloads, 
	const sockaddr_in& clientAddress)
{
	short responseCode;

	//Below snippet to calculate elapsed time taken from https://stackoverflow.com/a/31657669
	auto now = chrono::high_resolution_clock::now();
	auto timeElapsedSinceLastImagePayloadRecd = chrono::duration_cast<chrono::milliseconds>(now - lastImagePayloadRecdTime);
	
	// cout << "\ntimeElapsedSinceLastImagePayloadRecd: " << timeElapsedSinceLastImagePayloadRecd.count();

	if (timeElapsedSinceLastImagePayloadRecd.count() < IMAGE_PAYLOAD_RECV_TIMEOUT_MILLIS) {
		return RESPONSE_SUCCESS;
	}
	
	vector<u_short> missingSeqNumbers = calculateMissingPayloadSeqNumbers(imagePayloadSeqMap, expectedNumberOfPayloads);
	if (missingSeqNumbers.size() > 0) {
		responseCode = sendServerResponse(SERVER_NEGATIVE_ACK, clientAddress, &missingSeqNumbers);
	}
	else {
		responseCode = RESPONSE_SUCCESS;
	}
	
	lastImagePayloadRecdTime = chrono::high_resolution_clock::now();

	if (responseCode == RESPONSE_FAILURE) {
		cout << "\nERROR: Could not send response to client.";
		return RESPONSE_FAILURE;
	}
	
	return RESPONSE_SUCCESS;
}

short UDPServer::InitializeImageDimensions(cv::Size& imageDimensions, std::queue<string>& clientQueue)
{
	long lastPayloadSize = 0, bytesRecd = 0;
	string imageSizeString = "";

	while (bytesRecd < 15) {

		cout << "\nInitializeImageDimensions::Before popping from queue.";

		/*if (clientQueue.empty()) {
			cout << "\nClient queue is empty before fetching payload size string.";
		}
		string payloadSizeString = clientQueue.front();
		cout << "\nPayload size string from queue: " << payloadSizeString;

		try {
			cout << "\nQueue size before popping: " << clientQueue.size();
			clientQueue.pop();
		}
		catch (exception& ex) {
			cout << "\nException while popping: " << ex.what();
		}
		

		

		try {
			lastPayloadSize = stol(payloadSizeString);
		}
		catch (invalid_argument iaexp) {
			cout << "\nERROR::Invalid message size string found in queue.";
			return RESPONSE_FAILURE;
		}*/

		if (clientQueue.empty()) {
			cout << "\nClient queue is empty before fetching image size payload.";
		}

		//string imageSizePayloadInQueue = clientQueue.front();
		imageSizeString += clientQueue.front();
		cout << "\nimageSizePayloadInQueue: " << imageSizeString;
		//strcpy_s(imageSizeString + bytesRecd, lastPayloadSize, clientQueue.front());
		//strcpy(imageSizeString + lastPayloadSize, clientQueue->front());
		clientQueue.pop();

		bytesRecd += imageSizeString.length();
		cout << "\nImage size string at iteration end: " << imageSizeString;
	}
	cout << "\nQueue size after initializing image dimensions: " << clientQueue.size();
	return processImageSizePayload(&imageSizeString[0], imageDimensions);
	
}

void UDPServer::processImageProcessingReq(char* receivedImageSizeData, const sockaddr_in clientAddress)
{
	cv::Size imageDimensions;
	short responseCode = processImageSizePayload(receivedImageSizeData, imageDimensions);
	short serverResponseCodeForClient = SERVER_POSITIVE_ACK;
	if (responseCode == RESPONSE_FAILURE) {
		serverResponseCodeForClient = SERVER_NEGATIVE_ACK;
	}
	responseCode = sendServerResponse(serverResponseCodeForClient, clientAddress, nullptr);
	if (responseCode == RESPONSE_FAILURE) {
		cout << "\nCould not send acknowledgement to client.";
		return;
	}
	cout << "\nAck sent.";

	responseCode = receiveImage(imageDimensions, clientAddress);
	if (responseCode == RESPONSE_FAILURE) {
		cout << "\nError while receiving image data from client.";
		return;
	}
}

short UDPServer::processImageSizePayload(char* receivedData, cv::Size& imageDimensions)
{
	cout << "\nImage size data recd from client: " << receivedData;
	const vector<std::string> splitImageSizeData = splitString(receivedData, ' ');

	//Data validity check
	if (splitImageSizeData.at(0) != SIZE_PAYLOAD_KEY || splitImageSizeData.size() < 3) {
		cout << "\nClient sent image size data in wrong format.";
		return RESPONSE_FAILURE;
	}
	try {
		imageDimensions = cv::Size(stoi(splitImageSizeData.at(1)), stoi(splitImageSizeData.at(2)));
	}
	catch (invalid_argument iaExp) {
		cout << "\nInvalid image size values received.";
		return RESPONSE_FAILURE;
	}

	return RESPONSE_SUCCESS;
}

vector<u_short> UDPServer::calculateMissingPayloadSeqNumbers(const map<u_short, string>& receivedPayloadsMap, u_short expectedNumberOfPayloads)
{
	vector<u_short> missingSeqNumbers;

	for (u_short i = 1; i <= expectedNumberOfPayloads; i++) {
		if (receivedPayloadsMap.count(i) == 0) {
			missingSeqNumbers.push_back(i);
		}
	}
	return missingSeqNumbers;
}

short UDPServer::sendServerResponse(short serverResponseCode, const sockaddr_in& clientAddress, const vector<u_short>* missingSeqNumbers)
{
	cout << "\nEntered sendAck.";
	_mtx.lock();
	if (_socket == INVALID_SOCKET) {
		cout << "\nERROR: Invalid socket.";
		_mtx.unlock();
		return RESPONSE_FAILURE;
		
	}

	string missingSeqNumbersString = "";
	if (missingSeqNumbers != nullptr) {
		for (const u_short& missingSeqNumber : *missingSeqNumbers) {
			missingSeqNumbersString.append(to_string(missingSeqNumber)).append(" ");
		}
	}
	string serverResponsePayload = string("RES ").append(to_string(serverResponseCode)).append(" ").append(missingSeqNumbersString).append("\0");
	cout << "\nServer response string: " << serverResponsePayload <<" | string length: "<<serverResponsePayload.length();

	//'\0' not counted in string.length(), hence adding 1 to the payload size parameter below.
	short bytesSent = sendto(_socket, &serverResponsePayload[0], serverResponsePayload.length()+1, 0, (const sockaddr*)&clientAddress, sizeof(clientAddress));
	_mtx.unlock();
	cout << "\nAfter unlocking socket in sendAck.";

	if (bytesSent <= 0) {
		cout << "\nError while sending acknowldgement to client. Error code: " << WSAGetLastError();
		return RESPONSE_FAILURE;
	}
	return RESPONSE_SUCCESS;
}

short UDPServer::receiveImage(const cv::Size& imageDimensions, const sockaddr_in& clientAddress)
{
	_mtx.lock();
	if (_socket == INVALID_SOCKET) {
		cout << "\nERROR: Invalid socket.";
		_mtx.unlock();
		return RESPONSE_FAILURE;
	}

	const sockaddr_in originalClientAddress = sockaddr_in(clientAddress);
	long imageSize = imageDimensions.width * imageDimensions.height * 3;
	char* recdImageData = new char[imageSize];
	int clientAddrSize = sizeof(clientAddress);
	long bytesRecd = 0, bytesLeftToReceive = imageSize;
	cout << "\nImage size before receiving: " << imageSize;
	while (bytesRecd < imageSize) {
		int bytesRecdThisIteration;
		char* dataRecdThisIteration = new char[60000];

		cout << "\nBytes left to receive: " << bytesLeftToReceive;

		if (bytesLeftToReceive >= 60000l) {
			//TODO try locking and unlocking socket here instead of for the whole loop
			bytesRecdThisIteration = recvfrom(_socket, dataRecdThisIteration, 60000l, 0, (sockaddr*)&clientAddress, &clientAddrSize);
		}
		else {
			bytesRecdThisIteration = recvfrom(_socket, dataRecdThisIteration, bytesLeftToReceive, 0, (sockaddr*)&clientAddress, &clientAddrSize);
		}
		
		if (bytesRecdThisIteration == SOCKET_ERROR) {
			int lastError = WSAGetLastError();
			//cout << "\nError in receiving image size data. Error code: " << lastError;

			if (lastError != WSAEWOULDBLOCK) {
				cout << "\nError in receiving image data. Error code: " << lastError;
				_mtx.unlock();
				return RESPONSE_FAILURE;
			}

			//cout << "\nNo data received in this iteration. Sleeping and retrying...";
			this_thread::sleep_for(chrono::milliseconds(50));

		}
		else {

			bytesRecd += bytesRecdThisIteration;
			bytesLeftToReceive -= bytesRecdThisIteration;
		}		
		
		if (originalClientAddress.sin_addr.s_addr != clientAddress.sin_addr.s_addr || originalClientAddress.sin_port != clientAddress.sin_port) {
			cout << "\nCLIENT ADDRESS CHANGED.";
		}
		cout << "\n Thread: "<<this_thread::get_id()<<" Bytes recd this iteration: " << bytesRecdThisIteration;
		cout << "\nOriginal client: " << originalClientAddress.sin_addr.s_addr << ":" << originalClientAddress.sin_port << " | clientAddress: " << clientAddress.sin_addr.s_addr << ":" << clientAddress.sin_port;
		delete[] dataRecdThisIteration;

	}
	_mtx.unlock();

	//TODO move all image processing out of here and check hash
	cout << "\nAll data recd. Re-shaping image now...";
	map<u_short, string> dummyMap;
	const Mat constructedImage = constructImageFromData(dummyMap, imageDimensions);

	ImageProcessor imageProcessor(constructedImage);
	imageProcessor.DisplayImage("Received image");
	imageProcessor.SaveImage(GetAddressToSaveImage());

	return RESPONSE_SUCCESS;

}

const Mat UDPServer::constructImageFromData(map<u_short, string> imageDataMap, const cv::Size& imageDimensions) {
	Mat recdImage = Mat(imageDimensions, CV_8UC3);
	cout << "\nConstructing image. Image data map size: " << imageDataMap.size();
	u_int numberOfImageFragments = imageDataMap.size(), currentImageFragment = 1;
	int currentImageFragmentByte = 0;
	const char* currentImageFragmentData = &(imageDataMap[currentImageFragment][0]);

	for (int i = 0; i < imageDimensions.height; i++) {
		for (int j = 0; j < imageDimensions.width; j++) {
			//cout << "\nInside reconstruction loop. i= " << i << " | j= " << j;
			if (currentImageFragmentByte >= 60000) {
				
				//cout << "\nCurrent fragment " << currentImageFragment << " completed. ";

				currentImageFragment++;
				currentImageFragmentData = &(imageDataMap[currentImageFragment][0]);
				currentImageFragmentByte = 0;
			}

			recdImage.at<Vec3b>(i, j) = Vec3b(*(currentImageFragmentData + currentImageFragmentByte),
				*(currentImageFragmentData + currentImageFragmentByte + 1), 
				*(currentImageFragmentData + currentImageFragmentByte + 2));
			currentImageFragmentByte += 3;
		}
	}

	cout << "\nImage re-shaped.";
	return recdImage;
}

const Mat UDPServer::constructImageFromData(const char* imageData, const cv::Size& imageDimensions) {
	Mat recdImage = Mat(imageDimensions, CV_8UC3);

	cout << "\nConstructing image from string. Image data length: " << strlen(imageData);

	for (int i = 0; i < imageDimensions.height; i++) {
		for (int j = 0; j < imageDimensions.width; j++) {
			recdImage.at<Vec3b>(i, j) = Vec3b(*(imageData), *(imageData + 1), *(imageData + 2));
			imageData += 3;
		}
	}

	cout << "\nImage re-shaped.";
	return recdImage;
}

bool UDPServer::isValid()
{
	_mtx.lock();
	if (_socket != INVALID_SOCKET) {
		_mtx.unlock();
		return true;
	}
	_mtx.unlock();
	return false;
	
}