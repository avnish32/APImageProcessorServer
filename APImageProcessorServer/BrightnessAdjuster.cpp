#include "BrightnessAdjuster.h"
#include<iostream>

using std::cout;
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

BrightnessAdjuster::BrightnessAdjuster(const Mat& sourceImage, const float& brightnessAdjFactor):ImageFilter(sourceImage)
{
	_brightnessAdjFactor = brightnessAdjFactor;
}

BrightnessAdjuster::~BrightnessAdjuster()
{
	cout << "\nDestroying BrightnessAdjuster.";
}

Mat BrightnessAdjuster::ApplyFilter()
{
	Mat targetImage = Mat(cv::Size(_sourceImage.cols, _sourceImage.rows), _sourceImage.type());

	Mat sourceBGRChannels[3];

	cv::split(_sourceImage, sourceBGRChannels);

	//Below formulae to adjust brightness taken from https://www.dfstudios.co.uk/articles/programming/image-programming-algorithms/image-processing-algorithms-part-4-brightness-adjustment/
	for (int i = 0; i < _sourceImage.rows; i++) {
		for (int j = 0; j < _sourceImage.cols; j++) {
			uchar red = round(_clampPixelValue(sourceBGRChannels[2].at<uchar>(i, j) * _brightnessAdjFactor, 0.0f, 255.0f));
			uchar green = round(_clampPixelValue(sourceBGRChannels[1].at<uchar>(i, j) * _brightnessAdjFactor, 0.0f, 255.0f));
			uchar blue = round(_clampPixelValue(sourceBGRChannels[0].at<uchar>(i, j) * _brightnessAdjFactor, 0.0f, 255.0f));
			
			targetImage.at<Vec3b>(i, j) = Vec3b(blue, green, red);
		}
	}
	
	return targetImage;
}
