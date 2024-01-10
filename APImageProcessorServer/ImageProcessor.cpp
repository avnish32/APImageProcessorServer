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
	msg_logger_->LogDebug("Image processor default constructor.");
	image_ = Mat(1, 1, CV_8UC1);
}

ImageProcessor::ImageProcessor(Mat image)
{
	image_ = image;
}

/*
Parameterized constructor; constructs an image having imageDimensions from the given imageDataMap.
*/
ImageProcessor::ImageProcessor(map<unsigned short, std::string> imageDataMap, const Size& imageDimensions, const uint& imageFileSize)
{
	short numOfChannels = imageFileSize / (imageDimensions.width * imageDimensions.height);
	
	msg_logger_->LogDebug("Inside ImageProcessor. Number of channels: " + to_string(numOfChannels));

	ImageConstructor* imageConstructor = ImageConstructorFactory::GetImageConstructor(numOfChannels, imageDataMap, imageDimensions);
	if (imageConstructor == nullptr) {
		msg_logger_->LogError("ERROR: Constructing image with " + to_string(numOfChannels) + "is currently not supported.");
		return;
	}
	
	image_ = imageConstructor->ConstructImage();
}

ImageProcessor::~ImageProcessor()
{
	msg_logger_->LogDebug("Image processor destructor.");
}

void ImageProcessor::DisplayImage(cv::String windowName)
{
	cv::namedWindow(windowName, cv::WINDOW_KEEPRATIO);
	imshow(windowName, image_);

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
	return imageFilter->ApplyFilter(image_);
}

void ImageProcessor::SaveImage(cv::String saveAddress)
{
	bool wasImageWritten = imwrite(saveAddress, image_);
	if (!wasImageWritten) {
		msg_logger_->LogError("ERROR: Image could not be written to file.");
		return;
	}
	msg_logger_->LogDebug("Image written to file successfully.");
}

void ImageProcessor::SaveImage(Mat imageToSave, cv::String saveAddress)
{
	bool wasImageWritten = imwrite(saveAddress, imageToSave);
	if (!wasImageWritten) {
		msg_logger_->LogError("ERROR: Image could not be written to file.");
		return;
	}
	msg_logger_->LogDebug("Image written to file successfully.");
}

Mat ImageProcessor::GetImage()
{
	return image_;
}