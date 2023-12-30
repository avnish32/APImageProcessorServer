#include "BrightnessAdjuster.h"

#include<iostream>
#include<string>

using std::cout;
using std::to_string;

using cv::Vec3b;

float BrightnessAdjuster::_clampPixelValue(float unClampedValue, const float minValue, const float maxValue)
{
	if (unClampedValue < minValue) {
		return minValue;
	}

	if (unClampedValue > maxValue) {
		return  maxValue;
	}
	
	return unClampedValue;
}

BrightnessAdjuster::BrightnessAdjuster()
{
	_brightnessAdjFactor = 1;
}

BrightnessAdjuster::BrightnessAdjuster(const float& brightnessAdjFactor)
{
	_brightnessAdjFactor = brightnessAdjFactor;
}

BrightnessAdjuster::~BrightnessAdjuster()
{
	//cout << "\nDestroying BrightnessAdjuster.";
	_msgLogger->LogDebug("Destroying BrightnessAdjuster.");
}

Mat BrightnessAdjuster::ApplyFilter(const Mat& sourceImage)
{
	//cout << "\nAdjusting brightness. Adjustment factor: "<<_brightnessAdjFactor;
	_msgLogger->LogError("Adjusting brightness. Adjustment factor: " + to_string(_brightnessAdjFactor));

	Mat targetImage = Mat(cv::Size(sourceImage.cols, sourceImage.rows), sourceImage.type());

	Mat sourceBGRChannels[3];

	cv::split(sourceImage, sourceBGRChannels);

	//Below formulae to adjust brightness taken from https://www.dfstudios.co.uk/articles/programming/image-programming-algorithms/image-processing-algorithms-part-4-brightness-adjustment/
	for (int i = 0; i < sourceImage.rows; i++) {
		for (int j = 0; j < sourceImage.cols; j++) {
			uchar red = round(_clampPixelValue(sourceBGRChannels[2].at<uchar>(i, j) * _brightnessAdjFactor, 0.0f, 255.0f));
			uchar green = round(_clampPixelValue(sourceBGRChannels[1].at<uchar>(i, j) * _brightnessAdjFactor, 0.0f, 255.0f));
			uchar blue = round(_clampPixelValue(sourceBGRChannels[0].at<uchar>(i, j) * _brightnessAdjFactor, 0.0f, 255.0f));
			
			targetImage.at<Vec3b>(i, j) = Vec3b(blue, green, red);
		}
	}
	
	return targetImage;
}
