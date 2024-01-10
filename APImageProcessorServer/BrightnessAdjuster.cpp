#include "BrightnessAdjuster.h"

#include<iostream>
#include<string>

using std::to_string;

using cv::Vec3b;

float BrightnessAdjuster::ClampPixelValue(float unClampedValue, const float minValue, const float maxValue)
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
	brightness_adj_factor_ = 1;
}

BrightnessAdjuster::BrightnessAdjuster(const float& brightnessAdjFactor)
{
	brightness_adj_factor_ = brightnessAdjFactor;
}

BrightnessAdjuster::~BrightnessAdjuster()
{
	msg_logger_->LogDebug("Destroying BrightnessAdjuster.");
}

/*
This function adjusts the brightness of the image by first splitting it into its constituent channels,
multiplying the pixel value in each channel by the brightness adjustment factor and 
then clamping the value between 0 and 255.
*/
Mat BrightnessAdjuster::ApplyFilter(const Mat& sourceImage)
{
	msg_logger_->LogError("Adjusting brightness. Adjustment factor: " + to_string(brightness_adj_factor_));

	Mat targetImage = Mat(cv::Size(sourceImage.cols, sourceImage.rows), sourceImage.type());

	Mat sourceBGRChannels[3];

	cv::split(sourceImage, sourceBGRChannels);

	//Below formulae to adjust brightness taken from https://www.dfstudios.co.uk/articles/programming/image-programming-algorithms/image-processing-algorithms-part-4-brightness-adjustment/
	for (int i = 0; i < sourceImage.rows; i++) {
		for (int j = 0; j < sourceImage.cols; j++) {
			uchar red = round(ClampPixelValue(sourceBGRChannels[2].at<uchar>(i, j) * brightness_adj_factor_, 0.0f, 255.0f));
			uchar green = round(ClampPixelValue(sourceBGRChannels[1].at<uchar>(i, j) * brightness_adj_factor_, 0.0f, 255.0f));
			uchar blue = round(ClampPixelValue(sourceBGRChannels[0].at<uchar>(i, j) * brightness_adj_factor_, 0.0f, 255.0f));
			
			targetImage.at<Vec3b>(i, j) = Vec3b(blue, green, red);
		}
	}
	
	return targetImage;
}
