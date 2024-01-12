#include "Resizer.h"

#include<opencv2/opencv.hpp>

#include<iostream>
#include<string>

using cv::Vec3b;

using std::to_string;

Resizer::Resizer()
{
	target_width_ = target_height_ = 1;
}

Resizer::Resizer(const u_short& target_width, const u_short& target_height)
{
	target_width_ = target_width;
	target_height_ = target_height;
}

Resizer::~Resizer()
{
	msg_logger_->LogDebug("Destroying Resizer.");
}

/*
This function resizes the sourceImage into an image having width - _targetWidth pixels and height = _targetHeight pixels.
It utilizes the nearest-neighbor algorithm to calculate the pixel value of the resultant image.
*/
Mat Resizer::ApplyFilter(const Mat& source_image)
{
	msg_logger_->LogError("Resizing image. Target width: " + to_string(target_width_) 
		+ " | Target height: " + to_string(target_height_));

	//Params reqd in payload: targetWidth, targetHeight
	//Nearest-neighbor algorithm used for resizing. Reference: https://courses.cs.vt.edu/~masc1044/L17-Rotation/ScalingNN.html
	u_short source_width = source_image.cols;
	u_short source_height = source_image.rows;

	if (source_width == target_width_ && source_height == target_height_) {
		return source_image;
	}
	
	Mat target_image = Mat(cv::Size(target_width_, target_height_), source_image.type());
	for (int i = 0; i < target_height_; i++) {
		for (int j = 0; j < target_width_; j++) {
			int source_image_row = round(((float)i / target_height_) * source_height);
			int source_image_col = round(((float)j / target_width_) * source_width);

			source_image_row = MIN(source_image_row, source_height - 1);
			source_image_col = MIN(source_image_col, source_width - 1);

			target_image.at<Vec3b>(i, j) = source_image.at<Vec3b>(source_image_row, source_image_col);
		}
	}

	return target_image;
}
