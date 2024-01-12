#include "RGBToGrayscaleConverter.h"
#include<iostream>>

using cv::split;

/*
This functions converts a coloured, RGB image into a grayscale image using the Luminosity method.
This involves splitting the sourceImage into its constituent channels and combining the weighted 
contribution of each channel into a single channel, where the weight of each channel is given by the formula:
grayscale = 0.3 * R + 0.59 * G + 0.11 * B
This formula was referred from https://www.baeldung.com/cs/convert-rgb-to-grayscale.
*/
Mat RGBToGrayscaleConverter::ApplyFilter(const Mat& source_image)
{
	msg_logger_->LogError("Converting from RGB to Grayscale.");

	//Initialize target image. Since this is a grayscale image, target need have only one channel
	Mat target_image = Mat(cv::Size(source_image.cols, source_image.rows), CV_8UC1);

	//Splitting RGB image into its constituent channels using Mat::split 
	// https://docs.opencv.org/2.4/modules/core/doc/operations_on_arrays.html#split
	Mat source_channels_split[3];
	split(source_image, source_channels_split);
	
	for (int i = 0; i < source_image.rows; i++) {
		for (int j = 0; j < source_image.cols; j++) {
			
			target_image.at<uchar>(i, j) = (0.3f * source_channels_split[2].at<uchar>(i, j))
				+ (0.59f * source_channels_split[1].at<uchar>(i, j))
				+ (0.11f * source_channels_split[0].at<uchar>(i, j));
		}
	}
	return target_image;
}
