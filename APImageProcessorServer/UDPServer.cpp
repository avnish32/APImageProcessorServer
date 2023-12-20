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
	cv::String imageSaveAddress = "./Resources/savedImage.jpg";
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
	//This invalidates the overloaded operator() of thread pool.
	ThreadPool threadPool(NUM_THREADS);
	//_threadPool(NUM_THREADS);

	//TODO infinite loop to keep listening to clients, and possibly a timeout to exit the loop
	short serverResponseCodeForClient = SERVER_POSITIVE_ACK;
	char* receivedData = new char[15]; //Size 9999 9999\0
	sockaddr_in clientAddress;
	int clientAddrSize = sizeof(clientAddress);
	short bytesRecd = 0;
	while (bytesRecd < 15) {
		short bytesRecdThisIteration = recvfrom(_socket, receivedData+bytesRecd, 15, 0, (sockaddr*)&clientAddress, &clientAddrSize);
		if (bytesRecdThisIteration <= 0) {
			cout << "\nError in receiving image size data. Error code: " << WSAGetLastError();
			sendAck(SERVER_NEGATIVE_ACK, clientAddress);
			return RESPONSE_FAILURE;
		}
		bytesRecd += bytesRecdThisIteration;
	}

	//TODO store future here and decide where to call future.get()
	threadPool.enqueue(bind(&UDPServer::processImageProcessingReq, this, receivedData, clientAddress));
	//return processImageSizePayload(receivedData);
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
			_mtx.unlock();
			return RESPONSE_FAILURE;
		}
		bytesRecd += bytesRecdThisIteration;
		bytesLeftToReceive -= bytesRecdThisIteration;
		
		cout << "\nBytes recd this iteration: " << bytesRecdThisIteration;
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