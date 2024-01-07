#include "Rotator.h"

#include<iostream>
#include<string>

using std::to_string;

using cv::Vec3b;

Mat Rotator::_RotateClockwiseOnce(const Mat& sourceImage)
{
	ushort sourceWidth = sourceImage.cols;
	ushort sourceHeight = sourceImage.rows;

	ushort targetWidth = sourceHeight;
	ushort targetHeight = sourceWidth;

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

Mat Rotator::_RotateAntiClockwiseOnce(const Mat& sourceImage)
{
	ushort sourceWidth = sourceImage.cols;
	ushort sourceHeight = sourceImage.rows;

	ushort targetWidth = sourceHeight;
	ushort targetHeight = sourceWidth;

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

Mat Rotator::_RotateTwice(const Mat& sourceImage)
{
	ushort sourceWidth = sourceImage.cols;
	ushort sourceHeight = sourceImage.rows;

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
	_direction = CLOCKWISE;
	_numOfTurns = 0;
}

Rotator::Rotator(const RotationDirection& rotationMode, const ushort& numOfTurns)
{
	_direction = rotationMode;
	_numOfTurns = numOfTurns;
}

Rotator::~Rotator()
{
	_msgLogger->LogDebug("Destroying Rotator.");
}

/*
This function determines the effective rotation based on the rotation direction
and number of turns, and then calls the appropriate function.
*/
Mat Rotator::ApplyFilter(const Mat& sourceImage)
{
	_msgLogger->LogError("Rotating image. Direction: " + to_string(_direction) + " | Num of turns: " + to_string(_numOfTurns));

	//Reference to rotate image: https://courses.cs.vt.edu/~masc1044/L17-Rotation/rotateScale.html
	_numOfTurns %= 4;

	switch (_numOfTurns) {
	case 1:
		switch (_direction)
		{
		case CLOCKWISE:
			return _RotateClockwiseOnce(sourceImage);
		default:
			return _RotateAntiClockwiseOnce(sourceImage);
		}
	case 2:
		//For 2 turns, direction doesn't matter.
		return _RotateTwice(sourceImage);
	case 3:
		switch (_direction)
		{
		case CLOCKWISE:
			return _RotateAntiClockwiseOnce(sourceImage);
		default:
			return _RotateClockwiseOnce(sourceImage);
		}
	default:
		return sourceImage;
	}

	return Mat();
}
