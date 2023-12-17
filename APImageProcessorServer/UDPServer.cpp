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
				cout << "\nSocked bound successfully.";
			}
		}
	}
}

UDPServer::~UDPServer()
{
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

short UDPServer::sendAck(short serverResponseCode)
{
	short bytesSent = sendto(_socket, (char*)&serverResponseCode, sizeof(serverResponseCode), 0, (const sockaddr*)&_clientAddress, sizeof(_clientAddress));
	if (bytesSent <= 0) {
		cout << "\nError while sending acknowldgement to client. Error code: " << WSAGetLastError();
		return RESPONSE_FAILURE;
	}
	return RESPONSE_SUCCESS;
}

//------------Member functions

short UDPServer::receiveImageSize()
{
	if (_socket == INVALID_SOCKET) {
		cout << "\nERROR: Invalid socket.";
		return RESPONSE_FAILURE;
	}

	_imageDimensions = cv::Size(1024, 768); //TODO receive this from client, preferably as a cv::size object - conversion not possible from char* to cv::Size
	char* receivedData = new char[15]; //Size 9999 9999\0
	int clientAddrSize = sizeof(_clientAddress);
	short bytesRecd = 0;
	while (bytesRecd < 15) {
		short bytesRecdThisIteration = recvfrom(_socket, receivedData+bytesRecd, 15, 0, (sockaddr*)&_clientAddress, &clientAddrSize);
		if (bytesRecdThisIteration <= 0) {
			cout << "\nError in receiving image size data. Error code: " << WSAGetLastError();
			return RESPONSE_FAILURE;
		}
		bytesRecd += bytesRecdThisIteration;
	}

	cout << "\nImage size data recd from client: " << receivedData;
	const vector<std::string> splitImageSizeData = splitString(receivedData, ' ');

	//Data validity check
	if (splitImageSizeData.at(0) != "Size" || splitImageSizeData.size() < 3) {
		cout << "\nClient sent image size data in wrong format.";
		return RESPONSE_FAILURE;
	}
	try {
		_imageDimensions = cv::Size(stoi(splitImageSizeData.at(1)), stoi(splitImageSizeData.at(2)));
	}
	catch (invalid_argument iaExp) {
		cout << "\nIncorrect image size values received.";
		return RESPONSE_FAILURE;
	}
	
	return RESPONSE_SUCCESS;
}

short UDPServer::receiveImage()
{
	if (_socket == INVALID_SOCKET) {
		cout << "\nERROR: Invalid socket.";
		return RESPONSE_FAILURE;
	}

	long imageSize = _imageDimensions.width * _imageDimensions.height * 3;
	char* recdImageData = new char[imageSize];
	int clientAddrSize = sizeof(_clientAddress);
	long bytesRecd = 0, bytesLeftToReceive = imageSize;
	cout << "\nImage size before receiving: " << imageSize;
	while (bytesRecd < imageSize) {
		int bytesRecdThisIteration;

		cout << "\nBytes left to receive: " << bytesLeftToReceive;

		if (bytesLeftToReceive >= 60000l) {
			bytesRecdThisIteration = recvfrom(_socket, recdImageData + bytesRecd, 60000l, 0, (sockaddr*)&_clientAddress, &clientAddrSize);
		}
		else {
			bytesRecdThisIteration = recvfrom(_socket, recdImageData + bytesRecd, bytesLeftToReceive, 0, (sockaddr*)&_clientAddress, &clientAddrSize);
		}
		
		if (bytesRecdThisIteration <= 0) {
			cout << "\nError in receiving image data. Error code: "<<WSAGetLastError();
			return RESPONSE_FAILURE;
		}
		bytesRecd += bytesRecdThisIteration;
		bytesLeftToReceive -= bytesRecdThisIteration;
		
		cout << "\nBytes recd this iteration: " << bytesRecdThisIteration;
	}

	//TODO move all image processing out of here and check hash
	cout << "\nAll data recd. Re-shaping image now...";
	const Mat constructedImage = constructImageFromData(recdImageData);
	displayImage(constructedImage);
	saveImage(constructedImage);

	return RESPONSE_SUCCESS;

}

bool UDPServer::isValid()
{
	return _socket != INVALID_SOCKET;
}

const Mat UDPServer::constructImageFromData(char* imageData) {
	Mat recdImage = Mat(_imageDimensions, CV_8UC3);
	//TODO check if below works
	//recdImage.data = (uchar*) recdImageData;
	//long recdImgData;
	for (int i = 0; i < _imageDimensions.height; i++) {
		for (int j = 0; j < _imageDimensions.width; j++) {
			recdImage.at<Vec3b>(i, j) = Vec3b(*(imageData), *(imageData + 1), *(imageData + 2));
			imageData += 3;
		}
	}

	cout << "\nImage re-shaped.";
	return recdImage;
}