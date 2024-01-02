#include "RGBToGrayscaleConverter.h"
#include<iostream>>

using std::cout;

using cv::split;

/*
This functions converts a coloured, RGB image into a grayscale image using the Luminosity method.
This involves splitting the sourceImage into its constituent channels and combining the weighted 
contribution of each channel into a single channel, where the weight of each channel is given by the formula:
grayscale = 0.3 * R + 0.59 * G + 0.11 * B
This formula was referred from https://www.baeldung.com/cs/convert-rgb-to-grayscale.
*/
Mat RGBToGrayscaleConverter::ApplyFilter(const Mat& sourceImage)
{
	//cout << "\nConverting from RGB to Grayscale.";
	_msgLogger->LogError("Converting from RGB to Grayscale.");

	//Initialize target image. Since this is a grayscale image, target need have only one channel
	Mat targetImage = Mat(cv::Size(sourceImage.cols, sourceImage.rows), CV_8UC1);

	//Splitting RGB image into its constituent channels using Mat::split 
	// https://docs.opencv.org/2.4/modules/core/doc/operations_on_arrays.html#split
	Mat sourceChannelsSplit[3];
	split(sourceImage, sourceChannelsSplit);
	
	for (int i = 0; i < sourceImage.rows; i++) {
		for (int j = 0; j < sourceImage.cols; j++) {
			
			targetImage.at<uchar>(i, j) = (0.3f * sourceChannelsSplit[2].at<uchar>(i, j))
				+ (0.59f * sourceChannelsSplit[1].at<uchar>(i, j))
				+ (0.11f * sourceChannelsSplit[0].at<uchar>(i, j));
		}
	}
	return targetImage;
}
