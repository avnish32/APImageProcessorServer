#include "ImageProcessor.h"

#include<iostream>

using cv::imwrite;
using cv::imshow;
using cv::Vec3b;

using std::cout;

ImageProcessor::ImageProcessor()
{
	cout << "\nImage processor default constructor.";
	_image = Mat(1, 1, CV_8UC1);
}

ImageProcessor::ImageProcessor(Mat image)
{
	_image = image;
}

ImageProcessor::ImageProcessor(map<unsigned short, std::string> imageDataMap, const Size& imageDimensions)
{
	_image = Mat(imageDimensions, CV_8UC3);
	cout << "\nConstructing image. Image data map size: " << imageDataMap.size();
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

	cout << "\nImage re-shaped.";
}

ImageProcessor::~ImageProcessor()
{
	cout << "\nImage processor destructor.";
}

void ImageProcessor::DisplayImage(cv::String windowName)
{
	//namedWindow(windowName, WINDOW_KEEPRATIO);
	imshow(windowName, _image);

	//cv::waitKey(0);
	//cv::destroyWindow(windowName);
}

void ImageProcessor::SaveImage(cv::String saveAddress)
{
	bool wasImageWritten = imwrite(saveAddress, _image);
	if (!wasImageWritten) {
		cout << "\nImage could not be written to file.";
		return;
	}
	cout << "\nImage written to file successfully.";
}

Mat ImageProcessor::GetImage()
{
	return _image;
}
