#include "BrightnessAdjuster.h"

#include<iostream>
#include<string>

using std::to_string;

using cv::Vec3b;

float BrightnessAdjuster::ClampPixelValue(float unclamped_value, const float min_value, const float max_value)
{
	if (unclamped_value < min_value) {
		return min_value;
	}

	if (unclamped_value > max_value) {
		return  max_value;
	}
	
	return unclamped_value;
}

BrightnessAdjuster::BrightnessAdjuster()
{
	brightness_adj_factor_ = 1;
}

BrightnessAdjuster::BrightnessAdjuster(const float& brightness_adj_factor)
{
	brightness_adj_factor_ = brightness_adj_factor;
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
Mat BrightnessAdjuster::ApplyFilter(const Mat& source_image)
{
	msg_logger_->LogError("Adjusting brightness. Adjustment factor: " + to_string(brightness_adj_factor_));

	Mat target_image = Mat(cv::Size(source_image.cols, source_image.rows), source_image.type());

	Mat source_bgr_channels[3];

	cv::split(source_image, source_bgr_channels);

	//Below formulae to adjust brightness taken from https://www.dfstudios.co.uk/articles/programming/image-programming-algorithms/image-processing-algorithms-part-4-brightness-adjustment/
	for (int i = 0; i < source_image.rows; i++) {
		for (int j = 0; j < source_image.cols; j++) {
			uchar red = round(ClampPixelValue(source_bgr_channels[2].at<uchar>(i, j) * brightness_adj_factor_, 0.0f, 255.0f));
			uchar green = round(ClampPixelValue(source_bgr_channels[1].at<uchar>(i, j) * brightness_adj_factor_, 0.0f, 255.0f));
			uchar blue = round(ClampPixelValue(source_bgr_channels[0].at<uchar>(i, j) * brightness_adj_factor_, 0.0f, 255.0f));
			
			target_image.at<Vec3b>(i, j) = Vec3b(blue, green, red);
		}
	}
	
	return target_image;
}
