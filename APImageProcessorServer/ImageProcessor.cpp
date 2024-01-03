#include "ImageProcessor.h"
#include "ImageFilter.h"
#include "ImageFilterFactory.h"
#include "ImageConstructorFactory.h"

#include<iostream>
#include<string>

using cv::imwrite;
using cv::imshow;

using std::to_string;

ImageProcessor::ImageProcessor()
{
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
	
	_msgLogger->LogDebug("Inside ImageProcessor. Number of channels: " + to_string(numOfChannels));

	ImageConstructor* imageConstructor = ImageConstructorFactory::GetImageConstructor(numOfChannels, imageDataMap, imageDimensions);
	if (imageConstructor == nullptr) {
		_msgLogger->LogError("ERROR: Constructing image with " + to_string(numOfChannels) + "is currently not supported.");
		return;
	}
	
	_image = imageConstructor->ConstructImage();
}

ImageProcessor::~ImageProcessor()
{
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

void ImageProcessor::SaveImage(cv::String saveAddress)
{
	bool wasImageWritten = imwrite(saveAddress, _image);
	if (!wasImageWritten) {
		_msgLogger->LogError("ERROR: Image could not be written to file.");
		return;
	}
	_msgLogger->LogDebug("Image written to file successfully.");
}

void ImageProcessor::SaveImage(Mat imageToSave, cv::String saveAddress)
{
	bool wasImageWritten = imwrite(saveAddress, imageToSave);
	if (!wasImageWritten) {
		_msgLogger->LogError("ERROR: Image could not be written to file.");
		return;
	}
	_msgLogger->LogDebug("Image written to file successfully.");
}

Mat ImageProcessor::GetImage()
{
	return _image;
}