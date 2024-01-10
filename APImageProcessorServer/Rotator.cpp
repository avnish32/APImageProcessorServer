#include "Rotator.h"

#include<iostream>
#include<string>

using std::to_string;

using cv::Vec3b;

Mat Rotator::RotateClockwiseOnce(const Mat& sourceImage)
{
	u_short sourceWidth = sourceImage.cols;
	u_short sourceHeight = sourceImage.rows;

	u_short targetWidth = sourceHeight;
	u_short targetHeight = sourceWidth;

	Mat targetImage = Mat(cv::Size(targetWidth, targetHeight), sourceImage.type());

	for (int i = 0; i < targetHeight; i++) {
		for (int j = 0; j < targetWidth; j++) {
			int sourceImageRow = sourceHeight - j - 1;
			int sourceImageCol = i;
			targetImage.at<Vec3b>(i, j) = sourceImage.at<Vec3b>(sourceImageRow, sourceImageCol);
		}
	}

	return targetImage;
}

Mat Rotator::RotateAntiClockwiseOnce(const Mat& sourceImage)
{
	u_short sourceWidth = sourceImage.cols;
	u_short sourceHeight = sourceImage.rows;

	u_short targetWidth = sourceHeight;
	u_short targetHeight = sourceWidth;

	Mat targetImage = Mat(cv::Size(targetWidth, targetHeight), sourceImage.type());

	for (int i = 0; i < targetHeight; i++) {
		for (int j = 0; j < targetWidth; j++) {
			int sourceImageRow = j;
			int sourceImageCol = sourceWidth - i - 1;
			targetImage.at<Vec3b>(i, j) = sourceImage.at<Vec3b>(sourceImageRow, sourceImageCol);
		}
	}

	return targetImage;
}

Mat Rotator::RotateTwice(const Mat& sourceImage)
{
	u_short sourceWidth = sourceImage.cols;
	u_short sourceHeight = sourceImage.rows;

	Mat targetImage = Mat(cv::Size(sourceWidth, sourceHeight), sourceImage.type());

	for (int i = 0; i < sourceHeight; i++) {
		for (int j = 0; j < sourceWidth; j++) {
			int sourceImageRow = sourceHeight - i - 1;
			int sourceImageCol = sourceWidth - j - 1;
			targetImage.at<Vec3b>(i, j) = sourceImage.at<Vec3b>(sourceImageRow, sourceImageCol);
		}
	}

	return targetImage;
}

Rotator::Rotator()
{
	direction_ = CLOCKWISE;
	num_of_turns_ = 0;
}

Rotator::Rotator(const RotationDirection& rotationMode, const u_short& numOfTurns)
{
	direction_ = rotationMode;
	num_of_turns_ = numOfTurns;
}

Rotator::~Rotator()
{
	msg_logger_->LogDebug("Destroying Rotator.");
}

/*
This function determines the effective rotation based on the rotation direction
and number of turns, and then calls the appropriate function.
*/
Mat Rotator::ApplyFilter(const Mat& sourceImage)
{
	msg_logger_->LogError("Rotating image. Direction: " + to_string(direction_) + " | Num of turns: " + to_string(num_of_turns_));

	//Reference to rotate image: https://courses.cs.vt.edu/~masc1044/L17-Rotation/rotateScale.html
	num_of_turns_ %= 4;

	switch (num_of_turns_) {
	case 1:
		switch (direction_)
		{
		case CLOCKWISE:
			return RotateClockwiseOnce(sourceImage);
		default:
			return RotateAntiClockwiseOnce(sourceImage);
		}
	case 2:
		//For 2 turns, direction doesn't matter.
		return RotateTwice(sourceImage);
	case 3:
		switch (direction_)
		{
		case CLOCKWISE:
			return RotateAntiClockwiseOnce(sourceImage);
		default:
			return RotateClockwiseOnce(sourceImage);
		}
	default:
		return sourceImage;
	}

	return Mat();
}
