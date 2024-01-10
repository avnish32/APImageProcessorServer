#include "UDPServer.h"
#include "Constants.h"
#include "ImageMetadataProcessor.h"
#include "ImageProcessor.h"

#pragma comment (lib, "ws2_32.lib")

using std::cout;
using std::to_string;
using std::queue;
using std::stringstream;
using std::invalid_argument;
using std::chrono::milliseconds;
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;

UDPServer::UDPServer()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) == -1) {
		msg_logger_->LogError("Error while initialising WSA.");
		socket_ = INVALID_SOCKET;
	}
	else {
		socket_ = socket(AF_INET, SOCK_DGRAM, 0);
		if (socket_ == INVALID_SOCKET) {
			msg_logger_->LogError("Error while creating socket. Error code: " + to_string(WSAGetLastError()));
		}
		else {
			msg_logger_->LogError("Socket created successfully.");

			u_long NON_BLOCKING_MODE_TRUE = 0;
			if (ioctlsocket(socket_, FIONBIO, &NON_BLOCKING_MODE_TRUE) == SOCKET_ERROR) {
				msg_logger_->LogError("ERROR: Could not set server socket to non-blocking mode.");
				socket_ = INVALID_SOCKET;
			}
			sockaddr_in serverAddress;
			serverAddress.sin_family = AF_INET;
			serverAddress.sin_addr.s_addr = ADDR_ANY;
			serverAddress.sin_port = SERVER_DEFAULT_PORT;
			
			if (bind(socket_, (const sockaddr*)&serverAddress, (int) sizeof(serverAddress)) == SOCKET_ERROR) {
				msg_logger_->LogError("Error while binding server socket. Error code: " + to_string(WSAGetLastError()));
				socket_ = INVALID_SOCKET;
			}
			else {
				msg_logger_->LogError("Server ready to receive requests!");
			}
		}
	}
}

UDPServer::~UDPServer()
{
	msg_logger_->LogError("UDPServer destructor called.");
	WSACleanup();
	closesocket(socket_);
}

/*
This is the main orchestrator function that calls all the other function in order of the steps to perform 
for processing the image. When a new client connects, this is the function that is enqueued as a task into
the thread pool. Each connected client has their own copy of this function running in a separate thred.
*/
void UDPServer::ProcessImageReq(const sockaddr_in& clientAddress)
{
	cout << "\n\n";
	msg_logger_->LogError("Started processing request from client.");

	cv::Size imageDimensions;
	uint imageFileSize;
	ImageFilterTypesEnum filterType;
	vector<float> filterParams;
	short responseCode;
	short serverResponseCodeForClient = SERVER_POSITIVE_ACK;

	string clientAddressKey = to_string(clientAddress.sin_addr.s_addr) + CLIENT_ADDRESS_KEY_DELIMITER + to_string(clientAddress.sin_port);
	mtx_.lock();
	queue<string>& clientQueue = client_queue_map_[clientAddressKey];
	mtx_.unlock();

	
	responseCode = InitializeImageMetadata(imageDimensions, imageFileSize, filterType, filterParams, clientQueue);
	if (responseCode == RESPONSE_FAILURE) {
		serverResponseCodeForClient = SERVER_NEGATIVE_ACK;
	}
	responseCode = SendServerResponseToClient(serverResponseCodeForClient, clientAddress, nullptr);
	if (responseCode == RESPONSE_FAILURE) {
		msg_logger_->LogError("ERROR: Could not send acknowledgement to client.");
		RemoveClientDataFromMap(clientAddressKey);
		return;
	}
	msg_logger_->LogError("Acknowledgement sent. Server response code: " + to_string(serverResponseCodeForClient));

	if (serverResponseCodeForClient == SERVER_NEGATIVE_ACK) {
		return;
	}

	long imageBytesRecd = 0, imageBytesLeftToReceive = imageFileSize;
	u_short expectedNumberOfPayloads = imageBytesLeftToReceive / MAX_IMAGE_DATA_BYTES_IN_ONE_PAYLOAD;
	if (imageBytesLeftToReceive % MAX_IMAGE_DATA_BYTES_IN_ONE_PAYLOAD > 0) {
		expectedNumberOfPayloads++;
	}

	//char* recdImageData = new char[imageBytesLeftToReceive];
	string imageDataString = EMPTY_STRING;
	map<u_short, string> imagePayloadSeqMap;
	
	responseCode = ConsumeImageDataFromClientQueue(clientQueue, imagePayloadSeqMap, expectedNumberOfPayloads,
		clientAddress, imageBytesLeftToReceive);

	if (responseCode == RESPONSE_FAILURE) {
		msg_logger_->LogError("Error while consuming image data from client queue.");
		RemoveClientDataFromMap(clientAddressKey);
		return;
	}
	
	msg_logger_->LogError("All image data received. Sending positive ACK to client.");

	responseCode = SendServerResponseToClient(SERVER_POSITIVE_ACK, clientAddress, nullptr);
	if (responseCode == RESPONSE_FAILURE) {
		msg_logger_->LogError("ERROR: Could not send ACK to client.");
		RemoveClientDataFromMap(clientAddressKey);
		return;
	}

	msg_logger_->LogError("Acknowledgement sent. Server response code: " + to_string(SERVER_POSITIVE_ACK));

	//Saving original received image
	ImageProcessor imageProcessor(imagePayloadSeqMap, imageDimensions, imageFileSize);
	//imageProcessor.SaveImage();

	//Apply filter to image
	Mat filteredImage = imageProcessor.ApplyFilter(filterType, filterParams);
	//imageProcessor.SaveImage(filteredImage);
	
	msg_logger_->LogError("Filter applied. Sending processed image metadata to client.");

	//Send filtered image dimensions
	responseCode = SendImageMetadataToClient(filteredImage, clientAddress);
	if (responseCode == RESPONSE_FAILURE) {
		msg_logger_->LogError("Could not send metadata of processed image to client.");
		RemoveClientDataFromMap(clientAddressKey);
		return;
	}

	//Receive ACK from client
	vector<string> clientResponseSplit;
	short clientResponseCode;
	responseCode = ConsumeAndValidateClientMsgFromQueue(clientQueue, clientResponseSplit, clientResponseCode);
	if (responseCode == RESPONSE_FAILURE) {
		msg_logger_->LogError("Error while receiving/validating client response.");
		RemoveClientDataFromMap(clientAddressKey);
		return;
	}

	if (clientResponseCode == CLIENT_NEGATIVE_ACK) {
		msg_logger_->LogError("ERROR: Client sent negative ack. Terminating connection.");
		RemoveClientDataFromMap(clientAddressKey);
		return;
	}

	msg_logger_->LogError("Positive ACK recd from client. Sending processed image...");

	//Send processed image until positive ACK recd
	responseCode = SendImage(filteredImage, clientAddress, clientQueue);
	if (responseCode == RESPONSE_FAILURE) {
		msg_logger_->LogError("Error while sending processed image back to client.");
	}
	else {
		msg_logger_->LogError("Processed image successfully sent back to client.");
	}
	RemoveClientDataFromMap(clientAddressKey);
}

/*
* ---------------------
* Utility functions
* ---------------------
*/

/*
* Splits inputString based on delimiter character until the character '\0' is encountered.
*/
const vector<std::string> UDPServer::SplitString(char* inputString, char delimiter) {
	std::string currentWord = EMPTY_STRING;
	vector<std::string> outputVector;
	char* charPtr = inputString;
	
	while (*(charPtr) != STRING_TERMINATING_CHAR) {
		char currentChar = *(charPtr);
		if (currentChar == delimiter) {
			outputVector.push_back(currentWord);
			currentWord = EMPTY_STRING;
		}
		else {
			currentWord += currentChar;
		}
		++charPtr;
	}
	if (currentWord.length() > 0) {
		outputVector.push_back(currentWord);
	}
	return outputVector;
}

/*
Splits inputString by the delimiter character until the splits reach numberOfSplits,
or the whole length of the string given by inputStringLength has been traversed,
whichever occurs earlier.
*/
const vector<string> UDPServer::SplitString(char* inputString, const char& delimiter, const int& numberOfSplits,
	const int& inputStringLength) {
	std::string currentWord = EMPTY_STRING;
	int numberOfCurrentSplits = 1;
	vector<string> outputVector;
	//char* charPtr = inputString;
	
	for (int i = 0; i < inputStringLength; i++) {
		char currentChar = *(inputString + i);
		if (currentChar == delimiter && numberOfCurrentSplits < numberOfSplits) {
			outputVector.push_back(currentWord);
			currentWord = EMPTY_STRING;
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

	if (outputVector.size() < numberOfSplits) {
		stringstream sStream;
		sStream << inputString;
		msg_logger_->LogError("ERROR: Unexpected split. Original string: " + sStream.str());
	}
	return outputVector;
}

/*
Checks if timeoutDuration (in milliseconds) has passed between now and when the last message was received, given by lastMsgRecdTime.
*/
bool UDPServer::HasRequestTimedOut(high_resolution_clock::time_point& lastMsgRecdTime, const u_short& timeoutDuration)
{
	//Below snippet to calculate elapsed time taken from https://stackoverflow.com/a/31657669
	auto now = high_resolution_clock::now();
	auto timeElapsedSinceLastMsgRecd = duration_cast<milliseconds>(now - lastMsgRecdTime);

	if (timeElapsedSinceLastMsgRecd.count() >= timeoutDuration) {
		return true;
	}

	return false;
}

/*
Drains the client queue and keeps appending queue messages to msgInQueue
until the queue becomes empty or a message is encountered in the queue having '\0'
as its last character, whichever occurs earlier.
*/
u_short UDPServer::DrainQueue(std::queue<std::string>& clientQueue, std::string& msgInQueue)
{
	u_short bytesRecd = 0;

	while (!clientQueue.empty() && !msgInQueue.ends_with(STRING_TERMINATING_CHAR)) {
		msg_logger_->LogDebug("DrainQueue::Before popping from queue.");

		msgInQueue += clientQueue.front();
		msg_logger_->LogDebug("Msg in queue: " + msgInQueue);
		clientQueue.pop();
		bytesRecd += msgInQueue.length();
	}

	return bytesRecd;
}

/*
Returns the sequence numbers present in the series 0 to expectedNumberOfPayloads
but not in the key set of receivedPayloadsMap. Basically returns the sequence numbers
that are not yet received from the client.
*/
vector<u_short> UDPServer::GetMissingPayloadSeqNumbers(const map<u_short, string>& receivedPayloadsMap, u_short expectedNumberOfPayloads)
{
	vector<u_short> missingSeqNumbers;

	for (u_short i = 1; i <= expectedNumberOfPayloads; i++) {
		if (receivedPayloadsMap.count(i) == 0) {
			missingSeqNumbers.push_back(i);
		}
	}
	return missingSeqNumbers;
}

void UDPServer::RemoveClientDataFromMap(const string& clientAddressKey)
{
	//Client cleanup
	mtx_.lock();
	client_queue_map_.erase(clientAddressKey);
	mtx_.unlock();
}

/*
Constructs a string key in a particular format using the client IP Address and port.
*/
const std::string UDPServer::MakeClientAddressKey(const sockaddr_in& clientAddress)
{
	return to_string(clientAddress.sin_addr.s_addr) + CLIENT_ADDRESS_KEY_DELIMITER + to_string(clientAddress.sin_port);
}

/*
Consumes all image metadata payloads from the client queue 
and initializes the image metadata - imageDimensions, filterType and filterParams
after successful validation.
*/
short UDPServer::InitializeImageMetadata(cv::Size& imageDimensions, uint& imageFileSize, ImageFilterTypesEnum& filterType,
	vector<float>& filterParams,
	std::queue<string>& clientQueue)
{
	long lastPayloadSize = 0, bytesRecd = 0;
	string imageMetadataString = EMPTY_STRING;

	DrainQueue(clientQueue, imageMetadataString);
	msg_logger_->LogDebug("Queue size after consuming image metadata: " + to_string((u_short)clientQueue.size()));
	return ProcessImageMetadataPayload(&imageMetadataString[0], imageDimensions, imageFileSize, filterType, filterParams);
}

/*
Splits the raw image metadata string received from client by 'space' character
and forwards the resulting vector for validation.
*/
short UDPServer::ProcessImageMetadataPayload(char* receivedData, cv::Size& imageDimensions, uint& imageFileSize,
	ImageFilterTypesEnum& filterTypeEnum, vector<float>& filterParams)
{
	stringstream sStream;
	sStream << receivedData;
	msg_logger_->LogError("Image meta data recd from client: " + sStream.str());

	const vector<std::string> splitImageMetadata = SplitString(receivedData, ' ');

	ImageMetadataProcessor imageMetadataProcessor(splitImageMetadata);
	return imageMetadataProcessor.ValidateImageMetadata(imageDimensions, imageFileSize, filterTypeEnum, filterParams);
}

/*
Fragments image into payloads of 60,000 bytes and builds a map of the image data
in the format "<sequence_number>" -> "SEQ <sequence number> SIZE <payload size in bytes> <image data>" without the quotes.
This increases reliability by keeping track of which payloads were successfully received by the client.
It also helps retrieve the data of the payloads not received by the client so they can be re-sent.
*/
void UDPServer::BuildImageDataPayloadMap(Mat image, map<u_short, string>& imageDataPayloadMap,
	map<u_short, u_short>& sequenceNumToPayloadSizeMap, vector<u_short>& sequenceNumbers)
{
	uint imageBytesLeft = image.elemSize() * image.total();
	uint imageBytesProcessed = 0;
	string payload;
	u_short payloadSize;
	u_short payloadSequenceNum = 1;
	auto imagePtr = image.data;

	while (imageBytesLeft > 0) {

		u_short imageBytesProcessedThisIteration = 0;
		if (imageBytesLeft >= MAX_IMAGE_DATA_BYTES_IN_ONE_PAYLOAD) {

			payload = string(SEQUENCE_PAYLOAD_KEY).append(SERVER_MSG_DELIMITER)
				.append(to_string(payloadSequenceNum)).append(SERVER_MSG_DELIMITER)
				.append(SIZE_PAYLOAD_KEY).append(SERVER_MSG_DELIMITER)
				.append(to_string(MAX_IMAGE_DATA_BYTES_IN_ONE_PAYLOAD)).append(SERVER_MSG_DELIMITER);

			payloadSize = payload.length();
			string imageData = string((char*)imagePtr + imageBytesProcessed, MAX_IMAGE_DATA_BYTES_IN_ONE_PAYLOAD);
			payload += imageData;

			imageBytesProcessedThisIteration = MAX_IMAGE_DATA_BYTES_IN_ONE_PAYLOAD;
		}
		else {
			payload = string(SEQUENCE_PAYLOAD_KEY).append(SERVER_MSG_DELIMITER)
				.append(to_string(payloadSequenceNum)).append(SERVER_MSG_DELIMITER)
				.append(SIZE_PAYLOAD_KEY).append(SERVER_MSG_DELIMITER)
				.append(to_string(imageBytesLeft)).append(SERVER_MSG_DELIMITER);

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

				//_msgLogger->LogDebug("Bytes processed: " + to_string(imageBytesProcessed));
	}
}

/*
---------------------
Validation functions
---------------------
*/

/*
* This function validates the general client response.
* The expected format for this kind of response is "RES <responseCode>" without the quotes.
* Dependng on the circumstances, this response can have more parameters, but never less.
*/
short UDPServer::ValidateClientResponse(std::vector<std::string>& clientResponseSplit, short& clientResponseCode)
{
	msg_logger_->LogDebug("Validating client response.");

	if (clientResponseSplit.size() < MIN_CLIENT_RESPONSE_PARAMS || clientResponseSplit.at(0) != RESPONSE_PAYLOAD_KEY) {
		msg_logger_->LogError("ERROR: Client response not in expected format. Split size: " + to_string((u_short)clientResponseSplit.size())
			+ " | First element : " + clientResponseSplit.at(0));
		return RESPONSE_FAILURE;
	}

	try {
		clientResponseCode = stoi(clientResponseSplit.at(1));
	}
	catch (invalid_argument) {
		msg_logger_->LogError("ERROR: Response code not a number. Recd response code: " + clientResponseSplit.at(1));
		return RESPONSE_FAILURE;
	}

	msg_logger_->LogDebug("Client response validation successful.");
	return RESPONSE_SUCCESS;
}

/*
* Validates the image data payload sent by the client.
* The expected format for this kind of message is: "SEQ <payload sequence number> SIZE <size of the image data in payload> <image data>"
* without the quotes.
*/
short UDPServer::ValidateImageDataPayload(std::vector<std::string>& splitImageDataPayload, u_int& payloadSeqNum, u_int& payloadSize)
{
	if (splitImageDataPayload.size() != NUM_OF_IMAGE_DATA_PARAMS || splitImageDataPayload.at(0) != SEQUENCE_PAYLOAD_KEY
		|| splitImageDataPayload.at(2) != SIZE_PAYLOAD_KEY) {
		msg_logger_->LogError("ERROR: Image data payload in incorrect format. First word: " + splitImageDataPayload.at(0));
		return RESPONSE_FAILURE;
	}

	try {
		payloadSeqNum = stoi(splitImageDataPayload.at(1));
		payloadSize = stoi(splitImageDataPayload.at(3));
	}
	catch (invalid_argument) {
		msg_logger_->LogError("ERROR: Image data payload sequence num or size not an int. Seq num: " + splitImageDataPayload.at(1)
			+ " | Size:" + splitImageDataPayload.at(3));
		return RESPONSE_FAILURE;
	}

	return RESPONSE_SUCCESS;
}

/*
* --------------------------------
* Functions to send data to client
* --------------------------------
*/

/*
* Sends response to the client in the format "RES <response code> <missing payload sequence numbers (if any)>"
* without the quotes.
* This response helps the client to ensure that server has received the message/s succuessfully.
*/
short UDPServer::SendServerResponseToClient(short serverResponseCode, const sockaddr_in& clientAddress, const vector<u_short>* missingSeqNumbers)
{
	msg_logger_->LogDebug("Entered UDPServer::SendServerResponseToClient.");

	mtx_.lock();
	if (socket_ == INVALID_SOCKET) {
		msg_logger_->LogError("ERROR: Invalid socket.");
		mtx_.unlock();
		return RESPONSE_FAILURE;
	}
	mtx_.unlock();

	string missingSeqNumbersString = EMPTY_STRING;
	if (missingSeqNumbers != nullptr) {
		for (const u_short& missingSeqNumber : *missingSeqNumbers) {
			missingSeqNumbersString.append(to_string(missingSeqNumber)).append(SERVER_MSG_DELIMITER);
		}
	}
	string serverResponsePayload = string(RESPONSE_PAYLOAD_KEY).append(SERVER_MSG_DELIMITER)
		.append(to_string(serverResponseCode)).append(SERVER_MSG_DELIMITER)
		.append(missingSeqNumbersString).append(EMPTY_STRING + STRING_TERMINATING_CHAR);

		msg_logger_->LogDebug("Server response string: " + serverResponsePayload + " | String length: " 
		+ to_string((u_short)serverResponsePayload.length()));

	//'\0' not counted in string.length(), hence adding 1 to the payload size parameter below.
	mtx_.lock();
	short bytesSent = sendto(socket_, &serverResponsePayload[0], serverResponsePayload.length() + 1, 
		0, (const sockaddr*)&clientAddress, sizeof(clientAddress));
	mtx_.unlock();

	if (bytesSent <= 0) {
		msg_logger_->LogError("Error while sending acknowldgement to client. Error code: " + to_string(WSAGetLastError()));
		return RESPONSE_FAILURE;
	}
	return RESPONSE_SUCCESS;
}

/*
This function constructs a vector of the payload sequence numbers that the server did not receive
from the client before the last timeout occurred. It then sends these sequence numbers along with
a negative response code to the client so client can re-send the missing payloads.
*/
short UDPServer::SendMissingPayloadSeqNumbersToClient(std::map<u_short, std::string>& imagePayloadSeqMap, 
	const u_short& expectedNumberOfPayloads,
	vector<u_short>& missingPayloadSeqNumbersInLastTimeout, const sockaddr_in& clientAddress)
{
	short responseCode;
	vector<u_short> missingSeqNumbersInThisTimeout = GetMissingPayloadSeqNumbers(imagePayloadSeqMap, expectedNumberOfPayloads);

	if (missingSeqNumbersInThisTimeout.size() > 0) {
		if (missingPayloadSeqNumbersInLastTimeout == missingSeqNumbersInThisTimeout) {
			//Client did not send any more payloads since last timeout. Assuming that it is inactive now.
			msg_logger_->LogError("Client is inactive.");
			return RESPONSE_FAILURE;
		}
		missingPayloadSeqNumbersInLastTimeout = missingSeqNumbersInThisTimeout;
		responseCode = SendServerResponseToClient(SERVER_NEGATIVE_ACK, clientAddress, &missingSeqNumbersInThisTimeout);
	}
	else {
		responseCode = RESPONSE_SUCCESS;
	}

	if (responseCode == RESPONSE_FAILURE) {
		msg_logger_->LogError("ERROR: Could not send response to client.");
		return RESPONSE_FAILURE;
	}

	return RESPONSE_SUCCESS;
}

/*
Constructs the image metadata string in the format "SIZE <image width> <image height> <image size in bytes>\0"
without the quotes. This function is called after server has finished applying the requested filter on the original
image and is ready to send the resultant image back to the client.
*/
short UDPServer::SendImageMetadataToClient(const Mat& image, const sockaddr_in& clientAddress)
{
	std::string imageMetadataPayload = string(SIZE_PAYLOAD_KEY).append(SERVER_MSG_DELIMITER)
		.append(to_string(image.cols)).append(SERVER_MSG_DELIMITER)
		.append(to_string(image.rows)).append(SERVER_MSG_DELIMITER)
		.append(to_string(image.total() * image.elemSize())).append("\0");

	mtx_.lock();

	if (socket_ == INVALID_SOCKET) {
		msg_logger_->LogError("ERROR: Invalid client socket.");
		mtx_.unlock();
		return RESPONSE_FAILURE;
	}
	mtx_.unlock();

	u_short payloadSize = imageMetadataPayload.length() + 1; //1 added for \0 character at the end
	msg_logger_->LogDebug("Image metadata payload before sending: " + imageMetadataPayload + " | Size: " + to_string(payloadSize));

	int bytesSent = 0;

	while (bytesSent < payloadSize) {

		mtx_.lock();
		int bytesSentThisIteration = sendto(socket_, &imageMetadataPayload[0] + bytesSent, payloadSize - bytesSent,
			0, (sockaddr*)&clientAddress, sizeof(clientAddress));
		mtx_.unlock();

		if (bytesSentThisIteration <= 0) {
			msg_logger_->LogError("Error while sending image size. Error code: " + to_string(WSAGetLastError()));
			return RESPONSE_FAILURE;
		}
		bytesSent += bytesSentThisIteration;
	}

	msg_logger_->LogError("Image metadata successfully sent to client.");
	return RESPONSE_SUCCESS;
}

/*
This function builds the payload map for imageToSend and sends all image payloads
to the client. It also listens for client response after sending each batch of payloads 
to ascertain the ones lost in transit. These payloads are then re-sent in the next iteration, 
and the process is repeated until a positive acknowledgment is received from the client, or 
the client disconnects, whichever occurs earlier.
*/
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
			msg_logger_->LogError("ERROR: Could not send image payloads to client.");
			return RESPONSE_FAILURE;
		}

		vector<string> clientResponseSplit;
		responseCode = ConsumeAndValidateClientMsgFromQueue(clientQueue, clientResponseSplit, clientResponseCode);
		if (responseCode == RESPONSE_FAILURE) {
			msg_logger_->LogError("Error while receiving/validating client response.");
			return RESPONSE_FAILURE;
		}

		if (clientResponseCode == CLIENT_NEGATIVE_ACK) {
			payloadSeqNumbersToSend.clear();
			for (int i = 2; i < clientResponseSplit.size(); i++) {
				try {
					payloadSeqNumbersToSend.push_back(stoi(clientResponseSplit.at(i)));
				}
				catch (invalid_argument) {
					msg_logger_->LogError("ERROR: Sequence number sent by client not a number. Received sequence number: " 
						+ clientResponseSplit.at(i));
				}
			}
		}
	}

	msg_logger_->LogError("All image payloads received by client.");

	return RESPONSE_SUCCESS;
}

/*
This function fetches the image data payload corresponding to the numbers in payloadSeqNumbersToSend from imageDataPayloadMap
and sends them to the client.
*/
short UDPServer::SendImageDataPayloadsBySequenceNumbers(map<u_short, string>& imageDataPayloadMap,
	map<u_short, u_short>& sequenceNumToPayloadSizeMap, const vector<u_short>& payloadSeqNumbersToSend, const sockaddr_in& clientAddress)
{
	for (u_short payloadSeqNumberToSend : payloadSeqNumbersToSend) {
		char* payloadToSend = &(imageDataPayloadMap[payloadSeqNumberToSend][0]);

		mtx_.lock();
		int sendResult = sendto(socket_, payloadToSend, sequenceNumToPayloadSizeMap[payloadSeqNumberToSend], 
			0, (const sockaddr*)&clientAddress, sizeof(clientAddress));
		mtx_.unlock();

		if (sendResult == SOCKET_ERROR) {
			msg_logger_->LogError("ERROR while sending image payload to client. Error code " + to_string(WSAGetLastError()));
			return RESPONSE_FAILURE;
		}

		//TODO remove after testing
		//vector<string> payloadSplit = SplitString(payloadToSend, ' ', 5, sequenceNumToPayloadSizeMap[payloadSeqNumberToSend]);

		/*_msgLogger->LogDebug("Sent payload #" + to_string(payloadSeqNumberToSend)
				+ " | Payload split: " + payloadSplit.at(0) + " | " + payloadSplit.at(1) + " | " + payloadSplit.at(2) + " | " + payloadSplit.at(3)
				+ " | Length of image payload: " + to_string((ushort)payloadSplit.at(4).length()));*/

		msg_logger_->LogDebug("Sent payload #" + to_string(payloadSeqNumberToSend));
	}

	msg_logger_->LogDebug("Image payloads sent to client.");
	return RESPONSE_SUCCESS;
}

/*
--------------------------------------
Functions to receive data from client
--------------------------------------
*/

/*
This functions drains the queue until a message is found having '\0' as its last character.
While consuming from the queue, it checks for timeout and returns if a timeout does occur,
assuming that the client is not active anymore.
Upon succesfully receiving a properly terminated message, it splits the raw string by 'space'
character and sends the resulting vector for validation.
*/
short UDPServer::ConsumeAndValidateClientMsgFromQueue(std::queue<std::string>& clientQueue,
	std::vector<cv::String>& clientResponseSplit, short& clientResponseCode)
{
	string clientResponseRaw = EMPTY_STRING;
	short responseCode = RESPONSE_SUCCESS;
	auto lastMsgConsumedTime = high_resolution_clock::now();

	while (!clientResponseRaw.ends_with(STRING_TERMINATING_CHAR)) {
		u_short bytesRecdThisIteration = DrainQueue(clientQueue, clientResponseRaw);
		if (bytesRecdThisIteration > 0) {
			lastMsgConsumedTime = high_resolution_clock::now();
			continue;
		}
		if (HasRequestTimedOut(lastMsgConsumedTime, CLIENT_MSG_RECV_TIMEOUT_MILLIS)) {
			//Client is inactive.
			msg_logger_->LogError("ERROR: Server timed out while waiting for client response.");
			return RESPONSE_FAILURE;
		}
	}

	msg_logger_->LogDebug("Response from client: " + clientResponseRaw);
	clientResponseSplit = SplitString(&clientResponseRaw[0], CLIENT_RESPONSE_DELIMITER);

	responseCode = ValidateClientResponse(clientResponseSplit, clientResponseCode);
	if (responseCode == RESPONSE_FAILURE) {
		msg_logger_->LogError("ERROR: Validation failed for client response.");
		return RESPONSE_FAILURE;
	}
	return RESPONSE_SUCCESS;
}

/*
This function consumes the image data from client queue until no more bytes are left to receive. The bytes left to receive
are calculated from an earlier client message. While popping from the queue, the function also checks if timeout has occurred
every time it finds the queue empty. If a timeout does occur, it assumes that the client is not active anymore and returns.
Each raw image payload is then split by 'space' character and validated, before updating the imagePayloadSeqMap with the newly
consumed payload.
*/
short UDPServer::ConsumeImageDataFromClientQueue(std::queue<std::string>& clientQueue, std::map<u_short, std::string>& imagePayloadSeqMap,
	const u_short& expectedNumberOfPayloads, const sockaddr_in& clientAddress, long& imageBytesLeftToReceive)
{
	long imageBytesRecd = 0;
	short responseCode = RESPONSE_FAILURE;
	auto lastImagePayloadRecdTime = high_resolution_clock::now();
	vector<u_short> missingPayloadSeqNumbers;

	while (imageBytesLeftToReceive > 0) {

		if (clientQueue.empty()) {

			if (HasRequestTimedOut(lastImagePayloadRecdTime, IMAGE_PAYLOAD_RECV_TIMEOUT_MILLIS)) {
				responseCode = SendMissingPayloadSeqNumbersToClient(imagePayloadSeqMap, expectedNumberOfPayloads,
					missingPayloadSeqNumbers, clientAddress);
				lastImagePayloadRecdTime = high_resolution_clock::now();
				if (responseCode == RESPONSE_FAILURE) {
					msg_logger_->LogError("Error while sending response to client on timeout.");
					return RESPONSE_FAILURE;
				}
			}
			continue;
		}

		if (clientQueue.front().length() < MIN_CLIENT_PAYLOAD_SIZE_BYTES) {
			msg_logger_->LogError("ERROR: Unexpected msg found in queue. Msg: " + clientQueue.front());
			clientQueue.pop();
		}

		msg_logger_->LogDebug("Queue msg size before splitting: " + to_string((u_short)clientQueue.front().length()));

		vector<string> splitImageDataPayload = SplitString(&(clientQueue.front()[0]), CLIENT_RESPONSE_DELIMITER, NUM_OF_IMAGE_DATA_PARAMS,
			clientQueue.front().length());

		msg_logger_->LogDebug("Split image payload size: " + to_string((u_short)splitImageDataPayload.size()));

		u_int payloadSeqNum = 0, payloadSize = 0;

		responseCode = ValidateImageDataPayload(splitImageDataPayload, payloadSeqNum, payloadSize);
		if (responseCode == RESPONSE_FAILURE) {
			msg_logger_->LogError("ERROR: Validation failed for image data payload.");
			return responseCode;
		}

		imagePayloadSeqMap[payloadSeqNum] = splitImageDataPayload.at(4);

		msg_logger_->LogDebug("Image data after splitting: " + splitImageDataPayload.at(0) + " | " + splitImageDataPayload.at(1) + " | "
			+ splitImageDataPayload.at(2) + " | " + splitImageDataPayload.at(3)
			+ " | Length of image data: " + to_string((u_short)splitImageDataPayload.at(4).length()));

		clientQueue.pop();

		msg_logger_->LogDebug("After popping image data payload from queue. Queue size: " + to_string((u_short)clientQueue.size()));

		imageBytesRecd += payloadSize;
		imageBytesLeftToReceive -= payloadSize;

		msg_logger_->LogDebug("Image bytes recd: " + to_string(imageBytesRecd)
			+ " | image bytes left to receive: " + to_string(imageBytesLeftToReceive));

		lastImagePayloadRecdTime = high_resolution_clock::now();
	}

	return RESPONSE_SUCCESS;
}

/*
This function runs in the main server thread and continuously listens for incoming messages.
For every new client, a separate queue is created and all subsequent messages from that client
are enqueued in the corresponding queue. The connected clients are tracked through a map, with 
the IP address and port of each client acting as a unique key, and its corresponding queue as its value.
All further processing for the client is done in a separate thread managed by the thread pool.
*/
short UDPServer::StartReceivingClientMsgs()
{
	if (!IsValid()) {
		msg_logger_->LogError("ERROR: Invalid socket.");
		return RESPONSE_FAILURE;
	}

	//Created local thread pool so threads will be joined before UDP Server gets destroyed.
	//TODO This invalidates the overloaded operator() of thread pool.
	ThreadPool threadPool(NUM_THREADS);
	sockaddr_in* clientAddress = new sockaddr_in;

	while (true) {
		char* receivedData = new char[MAX_CLIENT_PAYLOAD_SIZE_BYTES];
		int clientAddrSize = sizeof(*clientAddress);

		long bytesRecdThisIteration = recvfrom(socket_, receivedData, MAX_CLIENT_PAYLOAD_SIZE_BYTES, 0, (sockaddr*)clientAddress, &clientAddrSize);

		if (bytesRecdThisIteration == SOCKET_ERROR) {
			int lastError = WSAGetLastError();

			if (lastError != WSAEWOULDBLOCK) {
				msg_logger_->LogError("Error in receiving data from client. Error code : " + to_string(lastError));
				SendServerResponseToClient(SERVER_NEGATIVE_ACK, *clientAddress, nullptr);
				RemoveClientDataFromMap(MakeClientAddressKey(*clientAddress));
				delete clientAddress;
				clientAddress = new sockaddr_in;
			}
		}
		else {
			msg_logger_->LogDebug("Image data recd from client.");
			string clientAddressKey = MakeClientAddressKey(*clientAddress);

			mtx_.lock();
			if (client_queue_map_.count(clientAddressKey) <= 0) {
				msg_logger_->LogDebug("Map entry not found for client address key: " + clientAddressKey);
				queue<string> newClientQueue;
				client_queue_map_[clientAddressKey] = newClientQueue;
				threadPool.Enqueue(bind(&UDPServer::ProcessImageReq, this, *clientAddress));
			}
			
			string imageData = string(receivedData, bytesRecdThisIteration);
			client_queue_map_[clientAddressKey].push(imageData);
			mtx_.unlock();
			
			msg_logger_->LogDebug("Image data size: " + to_string((u_short) imageData.length()));
			msg_logger_->LogDebug("Image data pushed to client queue. bytesRecdThisIterationString: " + to_string(bytesRecdThisIteration));
		}
		delete[] receivedData;
	}

	delete clientAddress;
	clientAddress = nullptr;
}

bool UDPServer::IsValid()
{
	mtx_.lock();
	if (socket_ != INVALID_SOCKET) {
		mtx_.unlock();
		return true;
	}
	mtx_.unlock();
	return false;
}