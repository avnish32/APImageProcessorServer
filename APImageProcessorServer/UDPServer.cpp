#include "UDPServer.h"
#include "Constants.h"
#include "ImageMetadataProcessor.h"
#include "ImageProcessor.h"

//TODO check if needed in final code
#include<format>

#pragma comment (lib, "ws2_32.lib")

//TODO substitute this
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
			_msgLogger->LogDebug("Socket created successfully.");

			u_long NON_BLOCKING_MODE_TRUE = 0;
			if (ioctlsocket(_socket, FIONBIO, &NON_BLOCKING_MODE_TRUE) == SOCKET_ERROR) {
				cout << "\nCould not set server socket to non-blocking mode.";
				_socket = INVALID_SOCKET;
			}
			sockaddr_in serverAddress;
			serverAddress.sin_family = AF_INET;
			serverAddress.sin_addr.s_addr = ADDR_ANY;
			serverAddress.sin_port = 8080;
			
			if (bind(_socket, (const sockaddr*)&serverAddress, (int) sizeof(serverAddress)) == SOCKET_ERROR) {
				cout << "\nError while binding server socket. Error code: " << WSAGetLastError();
				_socket = INVALID_SOCKET;
			}
			else {
				cout << "\nSocket bound successfully.";
				_msgLogger->LogDebug("Socket bound successfully.");
			}
		}
	}
}

UDPServer::~UDPServer()
{
	cout << "\nUDPServer destructor called.";
	WSACleanup();
	closesocket(_socket);
}

const vector<std::string> UDPServer::SplitString(char* inputString, char delimiter) {
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

const vector<string> UDPServer::SplitString(char* inputString, const char& delimiter, const int& numberOfSplits, const int& inputStringLength) {
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
		//cout << "\nUnexpected split. Original string: " << inputString;
		stringstream sStream;
		sStream << inputString;
		_msgLogger->LogError("ERROR: Unexpected split. Original string: " + sStream.str());
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

// Server maintains a map to store messages received from each client.
// For each new client, a new queue is created and a new task is pushed to be picked up by the thread pool.
// The main thread is responsible only for receiving messages from clients and pushing them
// into the client's respective queue.
// The client task then consumes and processes the messages from the queue.

short UDPServer::ReceiveClientMsg()
{
	if (_socket == INVALID_SOCKET) {
		_msgLogger->LogError("ERROR: Invalid socket.");
		return RESPONSE_FAILURE;
	}

	//Created local thread pool so threads will be joined before UDP Server gets destroyed.
	//TODO This invalidates the overloaded operator() of thread pool.
	ThreadPool threadPool(NUM_THREADS);

	while (true) {
		char* receivedData = new char[60025];
		sockaddr_in clientAddress;
		int clientAddrSize = sizeof(clientAddress);

		long bytesRecdThisIteration = recvfrom(_socket, receivedData, 60025L, 0, (sockaddr*)&clientAddress, &clientAddrSize);

		if (bytesRecdThisIteration == SOCKET_ERROR) {
			int lastError = WSAGetLastError();

			if (lastError != WSAEWOULDBLOCK) {
				_msgLogger->LogError("Error in receiving image size data. Error code : " + to_string(lastError));
				SendServerResponseToClient(SERVER_NEGATIVE_ACK, clientAddress, nullptr);
				return RESPONSE_FAILURE;
			}
		}
		else {
			//cout << "\nImage data recd from client.";
			_msgLogger->LogDebug("Image data recd from client.");
			string clientAddressKey = to_string(clientAddress.sin_addr.s_addr) + ":;" + to_string(clientAddress.sin_port);

			_mtx.lock();
			if (_clientToQueueMap.count(clientAddressKey) <= 0) {
				//cout << "\nMap entry not found for client address key: " << clientAddressKey;
				_msgLogger->LogDebug("Map entry not found for client address key: " + clientAddressKey);
				queue<string> newClientQueue;
				_clientToQueueMap[clientAddressKey] = newClientQueue;
				threadPool.enqueue(bind(&UDPServer::ProcessImageReq, this, clientAddress));
			}
			
			string imageData = string(receivedData, bytesRecdThisIteration);
			_clientToQueueMap[clientAddressKey].push(imageData);
			_mtx.unlock();

			//cout << "\nImage data size: " << imageData.length();
			_msgLogger->LogDebug("Image data size: " + to_string((ushort) imageData.length()));

			//cout << "\nImage data pushed to client queue. bytesRecdThisIterationString: "<< to_string(bytesRecdThisIteration);
			_msgLogger->LogDebug("Image data pushed to client queue. bytesRecdThisIterationString: " + to_string(bytesRecdThisIteration));
		}
		delete[] receivedData;
	}
}

void UDPServer::ProcessImageReq(const sockaddr_in& clientAddress)
{
	//cout << "\nThread: " << this_thread::get_id()<<" inside processImageReq.";
	cout << "\n\n";
	_msgLogger->LogError("Started processing request from client.");

	cv::Size imageDimensions;
	uint imageFileSize;
	ImageFilterTypesEnum filterType;
	vector<float> filterParams;
	short responseCode;
	short serverResponseCodeForClient = SERVER_POSITIVE_ACK;

	string clientAddressKey = to_string(clientAddress.sin_addr.s_addr) + ":;" + to_string(clientAddress.sin_port);
	_mtx.lock();
	queue<string>& clientQueue = _clientToQueueMap[clientAddressKey];
	_mtx.unlock();

	//cout << "\nClient queue size: " << clientQueue.size();

	responseCode = InitializeImageMetadata(imageDimensions, imageFileSize, filterType, filterParams, clientQueue);
	if (responseCode == RESPONSE_FAILURE) {
		serverResponseCodeForClient = SERVER_NEGATIVE_ACK;
	}
	responseCode = SendServerResponseToClient(serverResponseCodeForClient, clientAddress, nullptr);
	if (responseCode == RESPONSE_FAILURE) {
		//cout << "\nCould not send acknowledgement to client.";
		_msgLogger->LogError("ERROR: Could not send acknowledgement to client.");
		_RemoveClientDataFromMap(clientAddressKey);
		return;
	}
	//cout << "\nAck sent.";
	_msgLogger->LogError("Acknowledgement sent. Server response code: " + to_string(serverResponseCodeForClient));

	if (serverResponseCodeForClient == SERVER_NEGATIVE_ACK) {
		return;
	}

	long imageBytesRecd = 0, imageBytesLeftToReceive = imageFileSize;
	u_short expectedNumberOfPayloads = imageBytesLeftToReceive / 60000;
	if (imageBytesLeftToReceive % 60000 > 0) {
		expectedNumberOfPayloads++;
	}

	//char* recdImageData = new char[imageBytesLeftToReceive];
	string imageDataString = "";
	map<u_short, string> imagePayloadSeqMap;
	
	responseCode = ConsumeImageDataFromClientQueue(clientQueue, imagePayloadSeqMap, expectedNumberOfPayloads,
		clientAddress, imageBytesLeftToReceive);

	if (responseCode == RESPONSE_FAILURE) {
		//cout << "\nError while consuming image data from client queue.";
		_msgLogger->LogError("Error while consuming image data from client queue.");
		_RemoveClientDataFromMap(clientAddressKey);
		return;
	}
	
	//cout << "\nAll image data received. Sending positive ACK to client.";
	_msgLogger->LogError("All image data received. Sending positive ACK to client.");

	responseCode = SendServerResponseToClient(SERVER_POSITIVE_ACK, clientAddress, nullptr);
	if (responseCode == RESPONSE_FAILURE) {
		cout << "\nCould not send ACK to client.";
		_msgLogger->LogError("ERROR: Could not send ACK to client.");
		_RemoveClientDataFromMap(clientAddressKey);
		return;
	}

	_msgLogger->LogError("Acknowledgement sent. Server response code: " + to_string(SERVER_POSITIVE_ACK));

	ImageProcessor imageProcessor(imagePayloadSeqMap, imageDimensions, imageFileSize);
	//imageProcessor.DisplayImage("Received Image");
	imageProcessor.SaveImage();

	//Apply filter to image
	Mat filteredImage = imageProcessor.ApplyFilter(filterType, filterParams);
	//imageProcessor.SaveImage(filteredImage);
	
	_msgLogger->LogError("Filter applied. Sending processed image metadata to client.");

	//Send filtered image dimensions
	responseCode = SendImageMetadataToClient(filteredImage, clientAddress);
	if (responseCode == RESPONSE_FAILURE) {
		cout << "\nCould not send metadata of processed image to client.";
		_RemoveClientDataFromMap(clientAddressKey);
		return;
	}

	//Receive ACK from client
	vector<string> clientResponseSplit;
	short clientResponseCode;
	responseCode = ConsumeAndValidateClientMsgFromQueue(clientQueue, clientResponseSplit, clientResponseCode);
	if (responseCode == RESPONSE_FAILURE) {
		//cout << "\nError while receiving/validation client response.";
		_msgLogger->LogError("Error while receiving/validating client response.");
		_RemoveClientDataFromMap(clientAddressKey);
		return;
	}

	if (clientResponseCode == CLIENT_NEGATIVE_ACK) {
		//cout << "\nClient sent negative ack. Terminating connection.";
		_msgLogger->LogError("ERROR: Client sent negative ack. Terminating connection.");
		_RemoveClientDataFromMap(clientAddressKey);
		return;
	}

	_msgLogger->LogError("Positive ACK recd from client. Sending processed image.");

	//Send processed image until positive ACK recd
	responseCode = SendImage(filteredImage, clientAddress, clientQueue);
	if (responseCode == RESPONSE_FAILURE) {
		//cout << "\nError while sending processed image back to client.";
		_msgLogger->LogError("Error while sending processed image back to client.");
	}
	else {
		//cout << "\nProcessed image successfully sent back to client.";
		_msgLogger->LogError("Processed image successfully sent back to client.");
	}
	_RemoveClientDataFromMap(clientAddressKey);
}

short UDPServer::ConsumeImageDataFromClientQueue(std::queue<std::string>& clientQueue, std::map<u_short, std::string>& imagePayloadSeqMap, 
	const u_short& expectedNumberOfPayloads, const sockaddr_in& clientAddress, long& imageBytesLeftToReceive)
{
	long imageBytesRecd = 0;
	short responseCode = RESPONSE_FAILURE;
	auto lastImagePayloadRecdTime = chrono::high_resolution_clock::now();
	while (imageBytesLeftToReceive > 0) {

		if (clientQueue.empty()) {
			responseCode = CheckForTimeout(lastImagePayloadRecdTime, imagePayloadSeqMap, expectedNumberOfPayloads, clientAddress);
			if (responseCode == RESPONSE_FAILURE) {
				//cout << "\nError when sending response to client on timeout.";
				_msgLogger->LogError("Error while sending response to client on timeout.");
				return RESPONSE_FAILURE;
			}
			continue;
		}

		/*if (clientQueue.empty()) {
			cout << "\nClient queue is empty before fetching image data.";
		}*/

		if (clientQueue.front().length() < MIN_CLIENT_PAYLOAD_SIZE_BYTES) {
			//cout << "\nUnexpected msg found in queue. Msg: " << clientQueue.front();
			_msgLogger->LogError("ERROR: Unexpected msg found in queue. Msg: " + clientQueue.front());
			clientQueue.pop();
		}

		//cout << "\nQueue msg size before splitting: " << clientQueue.front().length();
		_msgLogger->LogDebug("Queue msg size before splitting: " + to_string((ushort) clientQueue.front().length()));

		vector<string> splitImageDataPayload = SplitString(&(clientQueue.front()[0]), ' ', 5, clientQueue.front().length());

		//cout << "\nSplit image payload size: " << splitImageDataPayload.size();
		_msgLogger->LogDebug("Split image payload size: " + to_string((ushort)splitImageDataPayload.size()));

		u_int payloadSeqNum = 0, payloadSize = 0;
		
		responseCode = ValidateImageDataPayload(splitImageDataPayload, payloadSeqNum, payloadSize);
		if (responseCode == RESPONSE_FAILURE) {
			//cout << "\nValidation failed for image data payload.";
			_msgLogger->LogError("ERROR: Validation failed for image data payload.");
			return responseCode;
		}

		imagePayloadSeqMap[payloadSeqNum] = splitImageDataPayload.at(4);

		/*cout << "\nImage data after splitting: " << splitImageDataPayload.at(0) << " | " << splitImageDataPayload.at(1) << " | "
			<< splitImageDataPayload.at(2) << " | " << splitImageDataPayload.at(3) << " | Length of image data: " << splitImageDataPayload.at(4).length();*/
		_msgLogger->LogDebug("Image data after splitting: " + splitImageDataPayload.at(0) + " | " + splitImageDataPayload.at(1) + " | "
			+ splitImageDataPayload.at(2) + " | " + splitImageDataPayload.at(3) 
			+ " | Length of image data: " + to_string((ushort)splitImageDataPayload.at(4).length()));

		clientQueue.pop();

		//cout << "\nAfter popping image data payload from queue. Queue size: " << clientQueue.size();
		_msgLogger->LogDebug("After popping image data payload from queue. Queue size: " + to_string((ushort)clientQueue.size()));

		imageBytesRecd += payloadSize;
		imageBytesLeftToReceive -= payloadSize;

		//cout << "\nImage bytes recd: " << imageBytesRecd << " | image bytes left to receive: " << imageBytesLeftToReceive;
		_msgLogger->LogDebug("Image bytes recd: " + to_string(imageBytesRecd) 
			+ " | image bytes left to receive: " + to_string(imageBytesLeftToReceive));

		lastImagePayloadRecdTime = chrono::high_resolution_clock::now();
	}
	
	return RESPONSE_SUCCESS;
}

short UDPServer::ValidateImageDataPayload(std::vector<std::string>& splitImageDataPayload, u_int& payloadSeqNum, u_int& payloadSize)
{
	//TODO shift hardcoded values to constants
	if (splitImageDataPayload.size() != 5 || splitImageDataPayload.at(0) != SEQUENCE_PAYLOAD_KEY 
		|| splitImageDataPayload.at(2) != SIZE_PAYLOAD_KEY) {
		//cout << "\nERROR: Image data payload in incorrect format. First word: " << splitImageDataPayload.at(0);
		_msgLogger->LogError("ERROR: Image data payload in incorrect format. First word: " + splitImageDataPayload.at(0));
		return RESPONSE_FAILURE;
	}

	try {
		payloadSeqNum = stoi(splitImageDataPayload.at(1));
		payloadSize = stoi(splitImageDataPayload.at(3));
	}
	catch (invalid_argument) {
		/*cout << "\nERROR: Image data payload sequence num or size not an int. Seq num: " << splitImageDataPayload.at(1)
			<< " | Size:" << splitImageDataPayload.at(3)*/;
		_msgLogger->LogError("ERROR: Image data payload sequence num or size not an int. Seq num: " + splitImageDataPayload.at(1)
			+ " | Size:" + splitImageDataPayload.at(3));
		return RESPONSE_FAILURE;
	}

	return RESPONSE_SUCCESS;
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
	//TODO keep a reference of missingSeqNumbers in prev call as a function param, compare it with this new vector.
	//If both are same, then client is not active anymore. Return failure response.
	if (missingSeqNumbers.size() > 0) {
		responseCode = SendServerResponseToClient(SERVER_NEGATIVE_ACK, clientAddress, &missingSeqNumbers);
	}
	else {
		responseCode = RESPONSE_SUCCESS;
	}
	
	lastImagePayloadRecdTime = chrono::high_resolution_clock::now();

	if (responseCode == RESPONSE_FAILURE) {
		//cout << "\nERROR: Could not send response to client.";
		_msgLogger->LogError("ERROR: Could not send response to client.");
		return RESPONSE_FAILURE;
	}
	
	return RESPONSE_SUCCESS;
}

short UDPServer::InitializeImageMetadata(cv::Size& imageDimensions, uint& imageFileSize, ImageFilterTypesEnum& filterType, vector<float>& filterParams,
	std::queue<string>& clientQueue)
{
	long lastPayloadSize = 0, bytesRecd = 0;
	string imageMetadataString = "";

	DrainQueue(clientQueue, imageMetadataString);

	//cout << "\nQueue size after consuming image metadata: " << clientQueue.size();
	_msgLogger->LogDebug("Queue size after consuming image metadata: " + to_string((ushort)clientQueue.size()));

	return ProcessImageMetadataPayload(&imageMetadataString[0], imageDimensions, imageFileSize, filterType, filterParams);
}

long UDPServer::DrainQueue(std::queue<std::string>& clientQueue, std::string& msgInQueue)
{
	long bytesRecd = 0;

	while (!clientQueue.empty()) {

		//cout << "\nDrainQueue::Before popping from queue.";
		_msgLogger->LogDebug("DrainQueue::Before popping from queue.");

		/*if (clientQueue.empty()) {
			cout << "\nClient queue is empty while draining.";
		}*/

		msgInQueue += clientQueue.front();
		
		//cout << "\nmsgInQueue: " << msgInQueue;
		_msgLogger->LogDebug("Msg in queue: " + msgInQueue);

		clientQueue.pop();

		bytesRecd += msgInQueue.length();
	}

	return bytesRecd;
}

void UDPServer::processImageProcessingReq(char* receivedImageSizeData, const sockaddr_in clientAddress)
{
	cv::Size imageDimensions;
	uint imageFileSize = 0;
	ImageFilterTypesEnum filterType;
	vector<float> filterParams;

	short responseCode = ProcessImageMetadataPayload(receivedImageSizeData, imageDimensions, imageFileSize, filterType, filterParams);
	short serverResponseCodeForClient = SERVER_POSITIVE_ACK;
	if (responseCode == RESPONSE_FAILURE) {
		serverResponseCodeForClient = SERVER_NEGATIVE_ACK;
	}
	responseCode = SendServerResponseToClient(serverResponseCodeForClient, clientAddress, nullptr);
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

short UDPServer::ProcessImageMetadataPayload(char* receivedData, cv::Size& imageDimensions, uint& imageFileSize,
	ImageFilterTypesEnum& filterTypeEnum, vector<float>& filterParams)
{
	//cout << "\nImage meta data recd from client: " << receivedData;
	stringstream sStream; 
	sStream << receivedData;
	_msgLogger->LogError("Image meta data recd from client: " + sStream.str());

	const vector<std::string> splitImageMetadata = SplitString(receivedData, ' ');

	ImageMetadataProcessor imageMetadataProcessor(splitImageMetadata);
	return imageMetadataProcessor.ValidateImageMetadata(imageDimensions, imageFileSize, filterTypeEnum, filterParams);
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

short UDPServer::SendServerResponseToClient(short serverResponseCode, const sockaddr_in& clientAddress, const vector<u_short>* missingSeqNumbers)
{
	//cout << "\nEntered UDPServer::SendServerResponseToClient.";
	_msgLogger->LogDebug("Entered UDPServer::SendServerResponseToClient.");

	_mtx.lock();
	if (_socket == INVALID_SOCKET) {
		
		cout << "\nERROR: Invalid socket.";
		_msgLogger->LogError("ERROR: Invalid socket.");

		_mtx.unlock();
		return RESPONSE_FAILURE;
	}
	_mtx.unlock();

	string missingSeqNumbersString = "";
	if (missingSeqNumbers != nullptr) {
		for (const u_short& missingSeqNumber : *missingSeqNumbers) {
			missingSeqNumbersString.append(to_string(missingSeqNumber)).append(" ");
		}
	}
	string serverResponsePayload = string(RESPONSE_PAYLOAD_KEY).append(SERVER_MSG_DELIMITER)
		.append(to_string(serverResponseCode)).append(SERVER_MSG_DELIMITER)
		.append(missingSeqNumbersString).append("\0");

	//cout << "\nServer response string: " << serverResponsePayload <<" | string length: "<<serverResponsePayload.length();
	_msgLogger->LogDebug("Server response string: " + serverResponsePayload + " | String length: " + to_string((ushort) serverResponsePayload.length()));

	//'\0' not counted in string.length(), hence adding 1 to the payload size parameter below.
	_mtx.lock();
	short bytesSent = sendto(_socket, &serverResponsePayload[0], serverResponsePayload.length()+1, 0, (const sockaddr*)&clientAddress, sizeof(clientAddress));
	_mtx.unlock();

	//TODO below check not working, bytes are sent even if client is not receiving
	if (bytesSent <= 0) {
		//cout << "\nError while sending acknowldgement to client. Error code: " << WSAGetLastError();
		_msgLogger->LogError("Error while sending acknowldgement to client. Error code: " + to_string(WSAGetLastError()));
		return RESPONSE_FAILURE;
	}
	return RESPONSE_SUCCESS;
}

short UDPServer::SendImageMetadataToClient(const Mat& image, const sockaddr_in& clientAddress)
{
	std::string imageMetadataPayload = string(SIZE_PAYLOAD_KEY).append(SERVER_MSG_DELIMITER)
		.append(to_string(image.cols)).append(SERVER_MSG_DELIMITER)
		.append(to_string(image.rows)).append(SERVER_MSG_DELIMITER)
		.append(to_string(image.total()*image.elemSize())).append("\0");

	_mtx.lock();
	if (_socket == INVALID_SOCKET) {
		//cout << "\nERROR: Invalid client socket.";
		_msgLogger->LogError("ERROR: Invalid client socket.");
		_mtx.unlock();
		return RESPONSE_FAILURE;
	}
	_mtx.unlock();

	ushort payloadSize = imageMetadataPayload.length() + 1; //1 added for \0 character at the end
	//cout << "\nImage metadata payload before sending: " << imageMetadataPayload << " | Size: " << payloadSize;
	_msgLogger->LogDebug("Image metadata payload before sending: " + imageMetadataPayload + " | Size: " + to_string(payloadSize));

	int bytesSent = 0;

	while (bytesSent < payloadSize) {

		_mtx.lock();
		int bytesSentThisIteration = sendto(_socket, &imageMetadataPayload[0] + bytesSent, payloadSize - bytesSent,
			0, (sockaddr*)&clientAddress, sizeof(clientAddress));
		_mtx.unlock();

		if (bytesSentThisIteration <= 0) {
			//cout << "\nError while sending image size. Error code: " << WSAGetLastError();
			_msgLogger->LogError("Error while sending image size. Error code: " + to_string(WSAGetLastError()));
			return RESPONSE_FAILURE;
		}
		bytesSent += bytesSentThisIteration;
	}

	//cout << "\nImage metadata successfully sent to client.";
	_msgLogger->LogError("Image metadata successfully sent to client.");
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
			//this_thread::sleep_for(chrono::milliseconds(50));

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
	//imageProcessor.DisplayImage("Received image");
	imageProcessor.SaveImage();

	return RESPONSE_SUCCESS;
}

short UDPServer::SendImage(const cv::Mat& imageToSend, const sockaddr_in& clientAddress, std::queue<std::string>& clientQueue)
{
	map<u_short, string> imageDataPayloadMap;
	map<u_short, u_short> sequenceNumToPayloadSizeMap;
	vector<u_short> payloadSeqNumbersToSend;
	short clientResponseCode = CLIENT_NEGATIVE_ACK;

	BuildImageDataPayloadMap(imageToSend, imageDataPayloadMap, sequenceNumToPayloadSizeMap, payloadSeqNumbersToSend);

	while (clientResponseCode != CLIENT_POSITIVE_ACK) {

		short responseCode = SendImageDataPayloadsBySequenceNumbers(imageDataPayloadMap, sequenceNumToPayloadSizeMap,
			payloadSeqNumbersToSend, clientAddress);
		if (responseCode == RESPONSE_FAILURE) {
			//cout << "\nCould not send image payloads to client.";
			_msgLogger->LogError("ERROR: Could not send image payloads to client.");
			return RESPONSE_FAILURE;
		}

		//TODO what if client disconnects and client response is never recd after server sends image payloads? 
		// Can check for timeout wherever we are consuming from queue. If no msg recd from client within timeout, destroy this thread.
		vector<string> clientResponseSplit;
		responseCode = ConsumeAndValidateClientMsgFromQueue(clientQueue, clientResponseSplit, clientResponseCode);
		if (responseCode == RESPONSE_FAILURE) {
			//cout << "\nError while receiving/validating client response.";
			_msgLogger->LogError("Error while receiving/validating client response.");
			return RESPONSE_FAILURE;
		}

		if (clientResponseCode == CLIENT_NEGATIVE_ACK) {
			payloadSeqNumbersToSend.clear();
			for (int i = 2; i < clientResponseSplit.size(); i++) {
				try {
					payloadSeqNumbersToSend.push_back(stoi(clientResponseSplit.at(i)));
				}
				catch (invalid_argument) {
					//cout << "\nERROR: Sequence number sent by client not a number. Received sequence number: " << clientResponseSplit.at(i);
					_msgLogger->LogError("ERROR: Sequence number sent by client not a number. Received sequence number: " + clientResponseSplit.at(i));
				}
			}
		}
	}

	//cout << "\nAll image payloads received by client.";
	_msgLogger->LogError("All image payloads received by client.");

	return RESPONSE_SUCCESS;
}

short UDPServer::ConsumeAndValidateClientMsgFromQueue(std::queue<std::string>& clientQueue, 
	std::vector<cv::String>& clientResponseSplit, short& clientResponseCode)
{
	string clientResponseRaw = "";
	short responseCode = RESPONSE_SUCCESS;

	while (!clientResponseRaw.ends_with('\0')) {
		DrainQueue(clientQueue, clientResponseRaw);
	}

	//cout << "\nResponse from client: " << clientResponseRaw;
	_msgLogger->LogDebug("Response from client: " + clientResponseRaw);

	clientResponseSplit = SplitString(&clientResponseRaw[0], CLIENT_RESPONSE_DELIMITER);

	responseCode = ValidateClientResponse(clientResponseSplit, clientResponseCode);
	if (responseCode == RESPONSE_FAILURE) {
		//cout << "\nERROR: Validation failed for client response.";
		_msgLogger->LogError("ERROR: Validation failed for client response.");
		return RESPONSE_FAILURE;
	}
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

void UDPServer::BuildImageDataPayloadMap(Mat image, map<u_short, string>& imageDataPayloadMap,
	map<u_short, u_short>& sequenceNumToPayloadSizeMap, vector<u_short>& sequenceNumbers)
{
	uint imageBytesLeft = image.elemSize() * image.total();
	uint imageBytesProcessed = 0;
	string payload;
	ushort payloadSize;
	ushort payloadSequenceNum = 1;
	auto imagePtr = image.data;

	while (imageBytesLeft > 0) {

		ushort imageBytesProcessedThisIteration = 0;
		if (imageBytesLeft >= 60000) {

			payload = string(SEQUENCE_PAYLOAD_KEY).append(" ").append(to_string(payloadSequenceNum)).append(" ")
				.append(SIZE_PAYLOAD_KEY).append(" ").append(to_string(60000)).append(" ");

			payloadSize = payload.length();
			string imageData = string((char*)imagePtr + imageBytesProcessed, 60000);
			payload += imageData;

			imageBytesProcessedThisIteration = 60000;
		}
		else {
			payload = string(SEQUENCE_PAYLOAD_KEY).append(" ").append(to_string(payloadSequenceNum)).append(" ")
				.append(SIZE_PAYLOAD_KEY).append(" ").append(to_string(imageBytesLeft)).append(" ");

			payloadSize = payload.length();
			string imageData = string((char*)imagePtr + imageBytesProcessed, imageBytesLeft);
			payload += imageData;

			imageBytesProcessedThisIteration = imageBytesLeft;
		}

		imageDataPayloadMap[payloadSequenceNum] = payload;

		payloadSize += imageBytesProcessedThisIteration;
		sequenceNumToPayloadSizeMap[payloadSequenceNum] = payloadSize;

		sequenceNumbers.push_back(payloadSequenceNum);
		imageBytesProcessed += imageBytesProcessedThisIteration;
		imageBytesLeft -= imageBytesProcessedThisIteration;

		payloadSequenceNum++;

		//cout << "\nBytes processd: " << imageBytesProcessed;
		_msgLogger->LogDebug("Bytes processed: " + to_string(imageBytesProcessed));
	}
}

short UDPServer::SendImageDataPayloadsBySequenceNumbers(map<u_short, string>& imageDataPayloadMap, 
	map<u_short, u_short>& sequenceNumToPayloadSizeMap,const vector<u_short>& payloadSeqNumbersToSend, const sockaddr_in& clientAddress)
{
	for (u_short payloadSeqNumberToSend : payloadSeqNumbersToSend) {
		char* payloadToSend = &(imageDataPayloadMap[payloadSeqNumberToSend][0]);

		_mtx.lock();
		int sendResult = sendto(_socket, payloadToSend, sequenceNumToPayloadSizeMap[payloadSeqNumberToSend], 0, (const sockaddr*)&clientAddress, sizeof(clientAddress));
		_mtx.unlock();

		if (sendResult == SOCKET_ERROR) {
			//cout << "\nERROR while sending image payload to client. Error code " << WSAGetLastError();
			_msgLogger->LogError("ERROR while sending image payload to client. Error code " + to_string(WSAGetLastError()));
			return RESPONSE_FAILURE;
		}
		
		//TODO remove after testing
		vector<string> payloadSplit = SplitString(payloadToSend, ' ', 5, sequenceNumToPayloadSizeMap[payloadSeqNumberToSend]);
		
		/*cout << "\nSent payload #" << payloadSeqNumberToSend
			<<" | Payload split: "<<payloadSplit.at(0)<<" | "<<payloadSplit.at(1)<<" | "<<payloadSplit.at(2)<<" | "<<payloadSplit.at(3)
			<<" | Length of image payload: "<<payloadSplit.at(4).length();*/

		_msgLogger->LogDebug("Sent payload #" + to_string(payloadSeqNumberToSend)
			+ " | Payload split: " + payloadSplit.at(0) + " | " + payloadSplit.at(1) + " | " + payloadSplit.at(2) + " | " + payloadSplit.at(3)
			+ " | Length of image payload: " + to_string((ushort)payloadSplit.at(4).length()));
	}

	//cout << "\nImage payloads sent to client.";
	_msgLogger->LogDebug("Image payloads sent to client.");

	return RESPONSE_SUCCESS;
}

void UDPServer::_RemoveClientDataFromMap(const string& clientAddressKey)
{
	//Client cleanup
	_mtx.lock();
	_clientToQueueMap.erase(clientAddressKey);
	_mtx.unlock();
}

short UDPServer::ValidateClientResponse(std::vector<std::string>& clientResponseSplit, short& clientResponseCode)
{
	//cout << "\nValidating client response.";
	_msgLogger->LogDebug("Validating client response.");

	if (clientResponseSplit.size() < 2 || clientResponseSplit.at(0) != RESPONSE_PAYLOAD_KEY) {
		//cout << "\nERROR: Client response not in expected format. Split size: " << serverResponseSplit.size();
		//cout << "\nFirst element: " << serverResponseSplit.at(0);
		_msgLogger->LogError("ERROR: Client response not in expected format. Split size: " + to_string((ushort)clientResponseSplit.size())
			+ " | First element : " + clientResponseSplit.at(0));
		return RESPONSE_FAILURE;
	}

	try {
		clientResponseCode = stoi(clientResponseSplit.at(1));
	}
	catch (invalid_argument) {
		//cout << "\nERROR: Response code not a number. Recd response code: " << serverResponseSplit.at(1);
		_msgLogger->LogError("ERROR: Response code not a number. Recd response code: " + clientResponseSplit.at(1));
		return RESPONSE_FAILURE;
	}

	//cout << "\nClient response validation successful.";
	_msgLogger->LogDebug("Client response validation successful.");
	return RESPONSE_SUCCESS;
}