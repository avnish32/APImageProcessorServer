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

Resizer::Resizer(const u_short& targetWidth, const u_short& targetHeight)
{
	target_width_ = targetWidth;
	target_height_ = targetHeight;
}

Resizer::~Resizer()
{
	msg_logger_->LogDebug("Destroying Resizer.");
}

/*
This function resizes the sourceImage into an image having width - _targetWidth pixels and height = _targetHeight pixels.
It utilizes the nearest-neighbor algorithm to calculate the pixel value of the resultant image.
*/
Mat Resizer::ApplyFilter(const Mat& sourceImage)
{
	msg_logger_->LogError("Resizing image. Target width: " + to_string(target_width_) 
		+ " | Target height: " + to_string(target_height_));

	//Params reqd in payload: targetWidth, targetHeight
	//Nearest-neighbor algorithm used for resizing. Reference: https://courses.cs.vt.edu/~masc1044/L17-Rotation/ScalingNN.html
	u_short sourceWidth = sourceImage.cols;
	u_short sourceHeight = sourceImage.rows;

	if (sourceWidth == target_width_ && sourceHeight == target_height_) {
		return sourceImage;
	}
	
	Mat targetImage = Mat(cv::Size(target_width_, target_height_), sourceImage.type());
	for (int i = 0; i < target_height_; i++) {
		for (int j = 0; j < target_width_; j++) {
			int sourceImageRow = round(((float)i / target_height_) * sourceHeight);
			int sourceImageCol = round(((float)j / target_width_) * sourceWidth);

			sourceImageRow = MIN(sourceImageRow, sourceHeight - 1);
			sourceImageCol = MIN(sourceImageCol, sourceWidth - 1);

			targetImage.at<Vec3b>(i, j) = sourceImage.at<Vec3b>(sourceImageRow, sourceImageCol);
		}
	}

	return targetImage;
}
