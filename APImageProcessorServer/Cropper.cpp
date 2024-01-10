#include "Cropper.h"

#include<iostream>
#include<string>

using std::to_string;

using cv::Vec3b;

Cropper::Cropper()
{
	top_left_corner_x_ = top_left_corner_y = 0;
	target_width_ = 1;
	target_height_ = 1;
}

Cropper::Cropper(const u_short& cropTopLeftCornerX, const u_short& cropTopLeftCornerY,
	const u_short& targetWidth, const u_short& targetHeight)
{
	top_left_corner_x_ = cropTopLeftCornerX;
	top_left_corner_y = cropTopLeftCornerY;
	target_width_ = targetWidth;
	target_height_ = targetHeight;
}

Cropper::~Cropper()
{
	msg_logger_->LogDebug("Cropper object destroyed.");
}

/*
This function crops the image by considering only the pixels covered inside the rectangle
starting at the coordinates (_cropTopLeftCornerX, _cropTopLeftCornerY) and having width of
_targetWidth pixels and height of _targetHeight pixels.
*/
Mat Cropper::ApplyFilter(const Mat& sourceImage)
{
	msg_logger_->LogError("Cropping image. Top left corner oordinates: (" + to_string(top_left_corner_x_) + "," 
		+ to_string(top_left_corner_y) + ") | Target width: " + to_string(target_width_) + " | Target height: " + to_string(target_height_));

	//Calculate bounds of the source image to process
	int sourceImageColLimit = MIN(top_left_corner_x_ + target_width_, sourceImage.cols);
	int sourceImageRowLimit = MIN(top_left_corner_y + target_height_, sourceImage.rows);
	
	//Initialize target dimensions
	target_width_ = sourceImageColLimit - top_left_corner_x_;
	target_height_ = sourceImageRowLimit - top_left_corner_y;	

	Mat targetImage = Mat(cv::Size(target_width_, target_height_), sourceImage.type());

	int currentTargetX = -1, currentTargetY = -1;

	for (int i = top_left_corner_y; i < sourceImageRowLimit; i++) {
		currentTargetY++;
		currentTargetX = 0;
		for (int j = top_left_corner_x_; j < sourceImageColLimit; j++) {
			targetImage.at<Vec3b>(currentTargetY, currentTargetX) = sourceImage.at<Vec3b>(i, j);
			currentTargetX++;
		}
	}

	return targetImage;
}
