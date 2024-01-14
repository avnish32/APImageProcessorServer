#include "Cropper.h"

#include<iostream>
#include<string>

using std::to_string;

using cv::Vec3b;

Cropper::Cropper()
{
	top_left_corner_x_ = top_left_corner_y_ = 0;
	target_width_ = 1;
	target_height_ = 1;
}

Cropper::Cropper(const u_short& top_left_corner_x, const u_short& top_left_corner_y,
	const u_short& target_width, const u_short& target_height)
{
	top_left_corner_x_ = top_left_corner_x;
	top_left_corner_y_ = top_left_corner_y;
	target_width_ = target_width;
	target_height_ = target_height;
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
Mat Cropper::ApplyFilter(const Mat& source_image)
{
	msg_logger_->LogError("Cropping image. Top left corner coordinates: (" + to_string(top_left_corner_x_) + "," 
		+ to_string(top_left_corner_y_) + ") | Target width: " + to_string(target_width_) + " | Target height: " + to_string(target_height_));

	//Calculate bounds of the source image to process
	int source_image_col_limit = MIN(top_left_corner_x_ + target_width_, source_image.cols);
	int source_image_row_limit = MIN(top_left_corner_y_ + target_height_, source_image.rows);
	
	//Initialize target dimensions
	target_width_ = source_image_col_limit - top_left_corner_x_;
	target_height_ = source_image_row_limit - top_left_corner_y_;	

	Mat target_image = Mat(cv::Size(target_width_, target_height_), source_image.type());

	int current_target_x = -1, current_target_y = -1;

	for (int i = top_left_corner_y_; i < source_image_row_limit; i++) {
		current_target_y++;
		current_target_x = 0;
		for (int j = top_left_corner_x_; j < source_image_col_limit; j++) {
			target_image.at<Vec3b>(current_target_y, current_target_x) = source_image.at<Vec3b>(i, j);
			current_target_x++;
		}
	}

	return target_image;
}
