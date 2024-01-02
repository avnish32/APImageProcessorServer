#include "ImageProcessor.h"
#include "ImageFilter.h"
#include "ImageFilterFactory.h"

#include<iostream>
#include<string>

using cv::imwrite;
using cv::imshow;
using cv::Vec2b;
using cv::Vec3b;
using cv::Vec4b;

using std::cout;
using std::to_string;

//TODO can choose to remove in the final build.
cv::String ImageProcessor::_GetAddressToSaveImage() {

	//Below snippet to convert thread id to string taken from https://stackoverflow.com/a/19255203
	auto threadId = std::this_thread::get_id();
	std::stringstream sStream;
	sStream << threadId;

	//Below snippet to convert chrono::time_point to string taken from https://stackoverflow.com/a/46240575
	//using namespace std::chrono_literals;
	std::chrono::time_point tp = std::chrono::system_clock::now();
	std::string timestamp = std::format("{:%H%M%S}", tp);

	//string timestamp = std::format("{:%H%M%s}", nowTime);
	cv::String imageSaveAddress = "./Resources/savedImage_" + sStream.str() + "_" + timestamp + ".jpg";
	return imageSaveAddress;
}

/*
This function constructs a single channel image having the given imageDimensions 
from the imageDataMap.
*/

//TODO use ImageConstructorFactory here
void ImageProcessor::_ConstructOneChannelImage(map<unsigned short, std::string> imageDataMap, const Size& imageDimensions)
{
	//cout << "\nConstructing one channel image. Image data map size: " << imageDataMap.size();
	_msgLogger->LogDebug("Constructing one channel image. Image data map size: " + to_string((ushort)imageDataMap.size()));

	_image = Mat(imageDimensions, CV_8UC1);
	unsigned short numberOfImageFragments = imageDataMap.size(), currentImageFragment = 1;
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

			_image.at<uchar>(i, j) = *(currentImageFragmentData + currentImageFragmentByte);
			currentImageFragmentByte += 1;
		}
	}

	//cout << "\nImage re-shaped.";
	_msgLogger->LogDebug("Image re-shaped.");
}

/*
This function constructs a double channel image having the given imageDimensions
from the imageDataMap.
*/
void ImageProcessor::_ConstructTwoChannelImage(map<unsigned short, std::string> imageDataMap, const Size& imageDimensions)
{
	//cout << "\nConstructing two channel image. Image data map size: " << imageDataMap.size();
	_msgLogger->LogDebug("Constructing two channel image. Image data map size: " + to_string((ushort)imageDataMap.size()));

	_image = Mat(imageDimensions, CV_8UC2);
	unsigned short numberOfImageFragments = imageDataMap.size(), currentImageFragment = 1;
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

			_image.at<Vec2b>(i, j) = Vec2b(*(currentImageFragmentData + currentImageFragmentByte),
				*(currentImageFragmentData + currentImageFragmentByte + 1));
			currentImageFragmentByte += 2;
		}
	}

	//cout << "\nImage re-shaped.";
	_msgLogger->LogDebug("Image re-shaped.");
}

/*
This function constructs a triple channel image having the given imageDimensions
from the imageDataMap.
*/
void ImageProcessor::_ConstructThreeChannelImage(map<unsigned short, std::string> imageDataMap, const Size& imageDimensions)
{
	//cout << "\nConstructing three channel image. Image data map size: " << imageDataMap.size();
	_msgLogger->LogDebug("Constructing three channel image. Image data map size: " + to_string((ushort)imageDataMap.size()));

	_image = Mat(imageDimensions, CV_8UC3);
	unsigned short numberOfImageFragments = imageDataMap.size(), currentImageFragment = 1;
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

			_image.at<Vec3b>(i, j) = Vec3b(*(currentImageFragmentData + currentImageFragmentByte),
				*(currentImageFragmentData + currentImageFragmentByte + 1),
				*(currentImageFragmentData + currentImageFragmentByte + 2));
			currentImageFragmentByte += 3;
		}
	}

	//cout << "\nImage re-shaped.";
	_msgLogger->LogDebug("Image re-shaped.");
}

/*
This function constructs a four channel image having the given imageDimensions
from the imageDataMap.
*/
void ImageProcessor::_ConstructFourChannelImage(map<unsigned short, std::string> imageDataMap, const Size& imageDimensions)
{
	//cout << "\nConstructing four channel image. Image data map size: " << imageDataMap.size();
	_msgLogger->LogDebug("Constructing four channel image. Image data map size: " + to_string((ushort)imageDataMap.size()));

	_image = Mat(imageDimensions, CV_8UC4);
	unsigned short numberOfImageFragments = imageDataMap.size(), currentImageFragment = 1;
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

			_image.at<Vec4b>(i, j) = Vec4b(*(currentImageFragmentData + currentImageFragmentByte),
				*(currentImageFragmentData + currentImageFragmentByte + 1),
				*(currentImageFragmentData + currentImageFragmentByte + 2),
				*(currentImageFragmentData + currentImageFragmentByte + 3));
			currentImageFragmentByte += 4;
		}
	}

	//cout << "\nImage re-shaped.";
	_msgLogger->LogDebug("Image re-shaped.");
}

ImageProcessor::ImageProcessor()
{
	//cout << "\nImage processor default constructor.";
	_msgLogger->LogDebug("Image processor default constructor.");
	_image = Mat(1, 1, CV_8UC1);
}

ImageProcessor::ImageProcessor(Mat image)
{
	_image = image;
}

/*
Parameterized constructor; constructs an image having imageDimensions from the given imageDataMap.
*/
ImageProcessor::ImageProcessor(map<unsigned short, std::string> imageDataMap, const Size& imageDimensions, const uint& imageFileSize)
{
	short numOfChannels = imageFileSize / (imageDimensions.width * imageDimensions.height);
	
	//cout << "\nInside ImageProcessor. Number of channels: " << numOfChannels;
	_msgLogger->LogDebug("Inside ImageProcessor. Number of channels: " + to_string(numOfChannels));

	switch (numOfChannels) {
	case 1:
		_ConstructOneChannelImage(imageDataMap, imageDimensions);
		break;
	case 2:
		_ConstructTwoChannelImage(imageDataMap, imageDimensions);
		break;
	case 3:
		_ConstructThreeChannelImage(imageDataMap, imageDimensions);
		break;
	case 4:
		_ConstructFourChannelImage(imageDataMap, imageDimensions);
		break;
	default:
		_ConstructThreeChannelImage(imageDataMap, imageDimensions);
		break;
	}
}

ImageProcessor::~ImageProcessor()
{
	//cout << "\nImage processor destructor.";
	_msgLogger->LogDebug("Image processor destructor.");
}

void ImageProcessor::DisplayImage(cv::String windowName)
{
	cv::namedWindow(windowName, cv::WINDOW_KEEPRATIO);
	imshow(windowName, _image);

	cv::waitKey(0);
	cv::destroyWindow(windowName);
}

/*
This function gets the appropriate image filter class depending on the filter type and then returns
the resultant image after applying the filter.
*/
Mat ImageProcessor::ApplyFilter(ImageFilterTypesEnum filterType, vector<float> filterParams)
{
	ImageFilter* imageFilter = ImageFilterFactory::GetImageFilter(filterType, filterParams);
	return imageFilter->ApplyFilter(_image);
}

//TODO can be removed in final build.
void ImageProcessor::SaveImage()
{
	bool wasImageWritten = imwrite(_GetAddressToSaveImage(), _image);
	if (!wasImageWritten) {
		//cout << "\nImage could not be written to file.";
		_msgLogger->LogError("ERROR: Image could not be written to file.");
		return;
	}
	//cout << "\nImage written to file successfully.";
	_msgLogger->LogDebug("Image written to file successfully.");
}

void ImageProcessor::SaveImage(cv::String saveAddress)
{
	bool wasImageWritten = imwrite(saveAddress, _image);
	if (!wasImageWritten) {
		_msgLogger->LogError("ERROR: Image could not be written to file.");
		return;
	}
	//cout << "\nImage written to file successfully.";
	_msgLogger->LogDebug("Image written to file successfully.");
}

//TODO can be removed in final build.
void ImageProcessor::SaveImage(Mat imageToSave)
{
	bool wasImageWritten = imwrite(_GetAddressToSaveImage(), imageToSave);
	if (!wasImageWritten) {
		_msgLogger->LogError("ERROR: Image could not be written to file.");
		return;
	}
	//cout << "\nImage written to file successfully.";
	_msgLogger->LogDebug("Image written to file successfully.");
}

void ImageProcessor::SaveImage(Mat imageToSave, cv::String saveAddress)
{
	bool wasImageWritten = imwrite(saveAddress, imageToSave);
	if (!wasImageWritten) {
		//cout << "\nImage could not be written to file.";
		_msgLogger->LogError("ERROR: Image could not be written to file.");
		return;
	}
	//cout << "\nImage written to file successfully.";
	_msgLogger->LogDebug("Image written to file successfully.");
}

Mat ImageProcessor::GetImage()
{
	return _image;
}