#include "Flipper.h"

using cv::Vec3b;

/*
This method flips the sourceImage horizontally by interchanging pixel values column-wise.
*/
Mat Flipper::_FlipHorizontally(const Mat& sourceImage)
{
	//Initialize target image
	Mat targetImage = Mat(cv::Size(sourceImage.cols, sourceImage.rows), sourceImage.type());

	for (int i = 0; i < sourceImage.rows; i++) {
		for (int j = 0; j < sourceImage.cols; j++) {
			targetImage.at<Vec3b>(i, j) = sourceImage.at<Vec3b>(i, sourceImage.cols - j - 1);
		}
	}

	return targetImage;
}

/*
This method flips the sourceImage vertically by interchanging the pixel values row-wise.
*/
Mat Flipper::_FlipVertically(const Mat& sourceImage)
{
	//Initialize target image
	Mat targetImage = Mat(cv::Size(sourceImage.cols, sourceImage.rows), sourceImage.type());

	for (int i = 0; i < sourceImage.rows; i++) {
		for (int j = 0; j < sourceImage.cols; j++) {
			targetImage.at<Vec3b>(i, j) = sourceImage.at<Vec3b>(sourceImage.rows - i - 1, j);
		}
	}

	return targetImage;
}

Flipper::Flipper()
{
	_flipDirection = HORIZONTAL;
}

Flipper::Flipper(const FlipDirection& flipDirection)
{
	_flipDirection = flipDirection;
}

Flipper::~Flipper()
{
	_msgLogger->LogDebug("Destroying Flipper object.");
}

Mat Flipper::ApplyFilter(const Mat& sourceImage)
{
	_msgLogger->LogError("Flipping image. Flip direction: " + _flipDirection);

	switch (_flipDirection) {
	case VERTICAL:
		return _FlipVertically(sourceImage);
	default:
		return _FlipHorizontally(sourceImage);
	}
}
