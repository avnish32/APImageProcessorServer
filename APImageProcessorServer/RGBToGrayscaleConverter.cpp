#include "RGBToGrayscaleConverter.h"
#include<iostream>>

using std::cout;

using cv::split;

RGBToGrayscaleConverter::RGBToGrayscaleConverter()
{
}

RGBToGrayscaleConverter::RGBToGrayscaleConverter(Mat sourceImage):ImageFilter(sourceImage)
{
}

RGBToGrayscaleConverter::~RGBToGrayscaleConverter()
{
	cout << "\nRGBToGrayscaleConverter object destroyed.";
}

Mat RGBToGrayscaleConverter::ApplyFilter()
{
	//Initialize target image. Since this is a grayscale image, target need have only one channel
	Mat targetImage = Mat(cv::Size(_sourceImage.cols, _sourceImage.rows), CV_8UC1);

	Mat sourceChannelsSplit[3];
	split(_sourceImage, sourceChannelsSplit);
	
	for (int i = 0; i < _sourceImage.rows; i++) {
		for (int j = 0; j < _sourceImage.cols; j++) {
			//Using Luminosity method to convert from RGB to Grayscale.
			// Below formula taken from https://www.baeldung.com/cs/convert-rgb-to-grayscale
			//grayscale = 0.3 * R + 0.59 * G + 0.11 * B}

			//Splitting RGB image into its constituent channels using Mat::split : https://docs.opencv.org/2.4/modules/core/doc/operations_on_arrays.html#split
			targetImage.at<uchar>(i, j) = (0.3f * sourceChannelsSplit[2].at<uchar>(i, j))
				+ (0.59f * sourceChannelsSplit[1].at<uchar>(i, j))
				+ (0.11f * sourceChannelsSplit[0].at<uchar>(i, j));
		}
	}
	return targetImage;
}
