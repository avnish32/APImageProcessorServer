#include "Flipper.h"

using cv::Vec3b;

/*
This method flips the sourceImage horizontally by interchanging pixel values column-wise.
*/
Mat Flipper::FlipHorizontally(const Mat& source_image)
{
	//Initialize target image
	Mat target_image = Mat(cv::Size(source_image.cols, source_image.rows), source_image.type());

	for (int i = 0; i < source_image.rows; i++) {
		for (int j = 0; j < source_image.cols; j++) {
			target_image.at<Vec3b>(i, j) = source_image.at<Vec3b>(i, source_image.cols - j - 1);
		}
	}

	return target_image;
}

/*
This method flips the sourceImage vertically by interchanging the pixel values row-wise.
*/
Mat Flipper::FlipVertically(const Mat& source_image)
{
	//Initialize target image
	Mat target_image = Mat(cv::Size(source_image.cols, source_image.rows), source_image.type());

	for (int i = 0; i < source_image.rows; i++) {
		for (int j = 0; j < source_image.cols; j++) {
			target_image.at<Vec3b>(i, j) = source_image.at<Vec3b>(source_image.rows - i - 1, j);
		}
	}

	return target_image;
}

Flipper::Flipper()
{
	flip_direction_ = HORIZONTAL;
}

Flipper::Flipper(const FlipDirection& flip_direction)
{
	flip_direction_ = flip_direction;
}

Flipper::~Flipper()
{
	msg_logger_->LogDebug("Destroying Flipper object.");
}

Mat Flipper::ApplyFilter(const Mat& source_image)
{
	msg_logger_->LogError("Flipping image. Flip direction: " + flip_direction_);

	switch (flip_direction_) {
	case VERTICAL:
		return FlipVertically(source_image);
	default:
		return FlipHorizontally(source_image);
	}
}
