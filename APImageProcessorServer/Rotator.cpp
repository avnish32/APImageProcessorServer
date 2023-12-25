#include "Rotator.h"
#include<iostream>

using std::cout;
using cv::Vec3b;

Mat Rotator::_RotateClockwiseOnce()
{
	u_short sourceWidth = _sourceImage.cols;
	u_short sourceHeight = _sourceImage.rows;

	u_short targetWidth = sourceHeight;
	u_short targetHeight = sourceWidth;

	Mat targetImage = Mat(cv::Size(targetWidth, targetHeight), _sourceImage.type());

	for (int i = 0; i < targetHeight; i++) {
		for (int j = 0; j < targetWidth; j++) {
			int sourceImageRow = sourceHeight - j - 1;
			int sourceImageCol = i;
			targetImage.at<Vec3b>(i, j) = _sourceImage.at<Vec3b>(sourceImageRow, sourceImageCol);
		}
	}

	return targetImage;
}

Mat Rotator::_RotateAntiClockwiseOnce()
{
	u_short sourceWidth = _sourceImage.cols;
	u_short sourceHeight = _sourceImage.rows;

	u_short targetWidth = sourceHeight;
	u_short targetHeight = sourceWidth;

	Mat targetImage = Mat(cv::Size(targetWidth, targetHeight), _sourceImage.type());

	for (int i = 0; i < targetHeight; i++) {
		for (int j = 0; j < targetWidth; j++) {
			int sourceImageRow = j;
			int sourceImageCol = sourceWidth - i - 1;
			targetImage.at<Vec3b>(i, j) = _sourceImage.at<Vec3b>(sourceImageRow, sourceImageCol);
		}
	}

	return targetImage;
}

Mat Rotator::_RotateTwice()
{
	u_short sourceWidth = _sourceImage.cols;
	u_short sourceHeight = _sourceImage.rows;

	Mat targetImage = Mat(cv::Size(sourceWidth, sourceHeight), _sourceImage.type());

	for (int i = 0; i < sourceHeight; i++) {
		for (int j = 0; j < sourceWidth; j++) {
			int sourceImageRow = sourceHeight - i - 1;
			int sourceImageCol = sourceWidth - j - 1;
			targetImage.at<Vec3b>(i, j) = _sourceImage.at<Vec3b>(sourceImageRow, sourceImageCol);
		}
	}

	return targetImage;
}

Rotator::Rotator()
{
	_direction = CLOCKWISE;
	_numOfTurns = 0;
}

Rotator::Rotator(const Mat& sourceImage, const u_short& numOfTurns, const RotationDirection& rotationMode):ImageFilter(sourceImage)
{
	_direction = rotationMode;
	_numOfTurns = numOfTurns;
}

Rotator::~Rotator()
{
	cout << "\ndestroying Rotator.";
}

Mat Rotator::ApplyFilter()
{
	//Reference to rotate image: https://courses.cs.vt.edu/~masc1044/L17-Rotation/rotateScale.html
	_numOfTurns %= 4;

	switch (_numOfTurns) {
	case 1:
		switch (_direction)
		{
		case CLOCKWISE:
			return _RotateClockwiseOnce();
		default:
			return _RotateAntiClockwiseOnce();
		}
	case 2:
		//For 2 turns, direction doesn't matter.
		return _RotateTwice();
	case 3:
		switch (_direction)
		{
		case CLOCKWISE:
			return _RotateAntiClockwiseOnce();
		default:
			return _RotateClockwiseOnce();
		}
	default:
		return _sourceImage;
	}

	return Mat();
}
