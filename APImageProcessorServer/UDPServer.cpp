#include "UDPServer.h"
#include "Constants.h"

#pragma comment (lib, "ws2_32.lib")

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

void displayImage(const Mat& image) {
	cv::String windowName = "Received Image";
	//namedWindow(windowName, WINDOW_KEEPRATIO);
	imshow(windowName, image);

	waitKey(0);
	destroyWindow(windowName);
}

void saveImage(const Mat& image) {
	auto threadId = this_thread::get_id();
	stringstream sStream;
	sStream << threadId;

	cv::String imageSaveAddress = "./Resources/savedImage_"+sStream.str()+".jpg";
	bool wasImageWritten = imwrite(imageSaveAddress, image);
	if (!wasImageWritten) {
		cout << "\nImage could not be written to file.";
		return;
	}
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
		char* receivedData = new char[60000];
		sockaddr_in clientAddress;
		int clientAddrSize = sizeof(clientAddress);

		//_mtx.lock();
		//cout << "\nWaiting for image size...";
		long bytesRecdThisIteration = recvfrom(_socket, receivedData, 60000L, 0, (sockaddr*)&clientAddress, &clientAddrSize);
		//_mtx.unlock();

		if (bytesRecdThisIteration == SOCKET_ERROR) {
			int lastError = WSAGetLastError();
			//cout << "\nError in receiving image size data. Error code: " << lastError;

			if (lastError != WSAEWOULDBLOCK) {
				cout << "\nError in receiving image size data. Error code: " << lastError;
				sendAck(SERVER_NEGATIVE_ACK, clientAddress);
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
			
			_clientToQueueMap[clientAddressKey].push(to_string(bytesRecdThisIteration));
			string imageData = string(receivedData, bytesRecdThisIteration);
			//cout << "\nImage data size: " << imageData.length();
			_clientToQueueMap[clientAddressKey].push(string(receivedData, bytesRecdThisIteration));
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

	responseCode = InitializeImageDimensions(imageDimensions, clientQueue);

	if (responseCode == RESPONSE_FAILURE) {
		serverResponseCodeForClient = SERVER_NEGATIVE_ACK;
	}
	responseCode = sendAck(serverResponseCodeForClient, clientAddress);
	if (responseCode == RESPONSE_FAILURE) {
		cout << "\nCould not send acknowledgement to client.";
		return;
	}
	cout << "\nAck sent.";

	long imageBytesRecd = 0, imageBytesLeftToReceive = imageDimensions.width * imageDimensions.height * 3;
	char* recdImageData = new char[imageBytesLeftToReceive];
	string imageDataString = "";

	while (imageBytesLeftToReceive > 0) {
		
		if (clientQueue.empty()) {
			continue;
		}
		
		long lastPayloadSize = 0;
		try {
			lastPayloadSize = stol(clientQueue.front());
			clientQueue.pop();
		}
		catch (invalid_argument) {
			cout << "\nERROR::Invalid payload size value found in queue.";
		}

		cout << "\nLast payload size for image data: " << lastPayloadSize;

		if (clientQueue.empty()) {
			cout << "\nClient queue is empty before fetching image data.";
		}
		//strcpy_s(recdImageData + imageBytesRecd, lastPayloadSize, clientQueue.front().c_str());
		imageDataString += clientQueue.front();
		clientQueue.pop();

		cout << "\nAfter popping image data payalod from queue. Queue size: "<<clientQueue.size();

		imageBytesRecd += lastPayloadSize;
		imageBytesLeftToReceive -= lastPayloadSize;		

		cout << "\nImage bytes recd: " << imageBytesRecd << " | image bytes left to receive: " << imageBytesLeftToReceive;
	}
	Mat constructedImage = constructImageFromData(&(imageDataString[0]), imageDimensions);
	//displayImage(constructedImage);
	saveImage(constructedImage);

	//TODO remove entry from map once processing completes for the client
}

short UDPServer::InitializeImageDimensions(cv::Size& imageDimensions, std::queue<string>& clientQueue)
{
	long lastPayloadSize = 0, bytesRecd = 0;
	string imageSizeString = "";

	while (bytesRecd < 15) {

		cout << "\nInitializeImageDimensions::Before popping from queue.";

		if (clientQueue.empty()) {
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
		}

		if (clientQueue.empty()) {
			cout << "\nClient queue is empty before fetching image size payload.";
		}

		//string imageSizePayloadInQueue = clientQueue.front();
		imageSizeString += clientQueue.front();
		cout << "\nimageSizePayloadInQueue: " << imageSizeString;
		//strcpy_s(imageSizeString + bytesRecd, lastPayloadSize, clientQueue.front());
		//strcpy(imageSizeString + lastPayloadSize, clientQueue->front());
		clientQueue.pop();

		bytesRecd += lastPayloadSize;
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
	responseCode = sendAck(serverResponseCodeForClient, clientAddress);
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
	if (splitImageSizeData.at(0) != "Size" || splitImageSizeData.size() < 3) {
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

short UDPServer::sendAck(short serverResponseCode, const sockaddr_in& clientAddress)
{
	cout << "\nEntered sendAck.";
	_mtx.lock();
	if (_socket == INVALID_SOCKET) {
		cout << "\nERROR: Invalid socket.";
		_mtx.unlock();
		return RESPONSE_FAILURE;
		
	}
	short bytesSent = sendto(_socket, (char*)&serverResponseCode, sizeof(serverResponseCode), 0, (const sockaddr*)&clientAddress, sizeof(clientAddress));
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
	const Mat constructedImage = constructImageFromData(recdImageData, imageDimensions);
	displayImage(constructedImage);
	saveImage(constructedImage);

	return RESPONSE_SUCCESS;

}

const Mat UDPServer::constructImageFromData(char* imageData, const cv::Size& imageDimensions) {
	Mat recdImage = Mat(imageDimensions, CV_8UC3);
	//TODO check if below works
	//recdImage.data = (uchar*) recdImageData;
	//long recdImgData;
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