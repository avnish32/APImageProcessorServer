#include "Flipper.h"

using cv::Vec3b;

/*
This method flips the sourceImage horizontally by interchanging pixel values column-wise.
*/
Mat Flipper::FlipHorizontally(const Mat& sourceImage)
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
Mat Flipper::FlipVertically(const Mat& sourceImage)
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
	flip_direction_ = HORIZONTAL;
}

Flipper::Flipper(const FlipDirection& flipDirection)
{
	flip_direction_ = flipDirection;
}

Flipper::~Flipper()
{
	msg_logger_->LogDebug("Destroying Flipper object.");
}

Mat Flipper::ApplyFilter(const Mat& sourceImage)
{
	msg_logger_->LogError("Flipping image. Flip direction: " + flip_direction_);

	switch (flip_direction_) {
	case VERTICAL:
		return FlipVertically(sourceImage);
	default:
		return FlipHorizontally(sourceImage);
	}
}
