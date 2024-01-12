#include "Rotator.h"

#include<iostream>
#include<string>

using std::to_string;

using cv::Vec3b;

Mat Rotator::RotateClockwiseOnce(const Mat& source_image)
{
	u_short source_width = source_image.cols;
	u_short source_height = source_image.rows;

	u_short target_width = source_height;
	u_short target_height = source_width;

	Mat target_image = Mat(cv::Size(target_width, target_height), source_image.type());

	for (int i = 0; i < target_height; i++) {
		for (int j = 0; j < target_width; j++) {
			int source_image_row = source_height - j - 1;
			int source_image_col = i;
			target_image.at<Vec3b>(i, j) = source_image.at<Vec3b>(source_image_row, source_image_col);
		}
	}

	return target_image;
}

Mat Rotator::RotateAntiClockwiseOnce(const Mat& source_image)
{
	u_short source_width = source_image.cols;
	u_short source_height = source_image.rows;

	u_short target_width = source_height;
	u_short target_height = source_width;

	Mat target_image = Mat(cv::Size(target_width, target_height), source_image.type());

	for (int i = 0; i < target_height; i++) {
		for (int j = 0; j < target_width; j++) {
			int source_image_row = j;
			int source_image_col = source_width - i - 1;
			target_image.at<Vec3b>(i, j) = source_image.at<Vec3b>(source_image_row, source_image_col);
		}
	}

	return target_image;
}

Mat Rotator::RotateTwice(const Mat& source_image)
{
	u_short source_width = source_image.cols;
	u_short source_height = source_image.rows;

	Mat target_image = Mat(cv::Size(source_width, source_height), source_image.type());

	for (int i = 0; i < source_height; i++) {
		for (int j = 0; j < source_width; j++) {
			int source_image_row = source_height - i - 1;
			int source_image_col = source_width - j - 1;
			target_image.at<Vec3b>(i, j) = source_image.at<Vec3b>(source_image_row, source_image_col);
		}
	}

	return target_image;
}

Rotator::Rotator()
{
	direction_ = CLOCKWISE;
	num_of_turns_ = 0;
}

Rotator::Rotator(const RotationDirection& rotation_mode, const u_short& no_of_turns)
{
	direction_ = rotation_mode;
	num_of_turns_ = no_of_turns;
}

Rotator::~Rotator()
{
	msg_logger_->LogDebug("Destroying Rotator.");
}

/*
This function determines the effective rotation based on the rotation direction
and number of turns, and then calls the appropriate function.
*/
Mat Rotator::ApplyFilter(const Mat& source_image)
{
	msg_logger_->LogError("Rotating image. Direction: " + to_string(direction_) + " | Num of turns: " + to_string(num_of_turns_));

	//Reference to rotate image: https://courses.cs.vt.edu/~masc1044/L17-Rotation/rotateScale.html
	num_of_turns_ %= 4;

	switch (num_of_turns_) {
	case 1:
		switch (direction_)
		{
		case CLOCKWISE:
			return RotateClockwiseOnce(source_image);
		default:
			return RotateAntiClockwiseOnce(source_image);
		}
	case 2:
		//For 2 turns, direction doesn't matter.
		return RotateTwice(source_image);
	case 3:
		switch (direction_)
		{
		case CLOCKWISE:
			return RotateAntiClockwiseOnce(source_image);
		default:
			return RotateClockwiseOnce(source_image);
		}
	default:
		return source_image;
	}

	return Mat();
}
