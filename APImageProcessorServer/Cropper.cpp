#include "Cropper.h"

#include<iostream>
#include<string>

using std::to_string;

using cv::Vec3b;

Cropper::Cropper()
{
	_cropTopLeftCornerX = _cropTopLeftCornerY = 0;
	_targetWidth = 1;
	_targetHeight = 1;
}

Cropper::Cropper(const ushort& cropTopLeftCornerX, const ushort& cropTopLeftCornerY,
	const ushort& targetWidth, const ushort& targetHeight)
{
	_cropTopLeftCornerX = cropTopLeftCornerX;
	_cropTopLeftCornerY = cropTopLeftCornerY;
	_targetWidth = targetWidth;
	_targetHeight = targetHeight;
}

Cropper::~Cropper()
{
	_msgLogger->LogDebug("Cropper object destroyed.");
}

/*
This function crops the image by considering only the pixels covered inside the rectangle
starting at the coordinates (_cropTopLeftCornerX, _cropTopLeftCornerY) and having width of
_targetWidth pixels and height of _targetHeight pixels.
*/
Mat Cropper::ApplyFilter(const Mat& sourceImage)
{
	_msgLogger->LogError("Cropping image. Top left corner oordinates: (" + to_string(_cropTopLeftCornerX) + "," 
		+ to_string(_cropTopLeftCornerY) + ") | Target width: " + to_string(_targetWidth) + " | Target height: " + to_string(_targetHeight));

	//Calculate bounds of the source image to process
	int sourceImageColLimit = MIN(_cropTopLeftCornerX + _targetWidth, sourceImage.cols);
	int sourceImageRowLimit = MIN(_cropTopLeftCornerY + _targetHeight, sourceImage.rows);
	
	//Initialize target dimensions
	_targetWidth = sourceImageColLimit - _cropTopLeftCornerX;
	_targetHeight = sourceImageRowLimit - _cropTopLeftCornerY;	

	Mat targetImage = Mat(cv::Size(_targetWidth, _targetHeight), sourceImage.type());

	int currentTargetX = -1, currentTargetY = -1;

	for (int i = _cropTopLeftCornerY; i < sourceImageRowLimit; i++) {
		currentTargetY++;
		currentTargetX = 0;
		for (int j = _cropTopLeftCornerX; j < sourceImageColLimit; j++) {
			targetImage.at<Vec3b>(currentTargetY, currentTargetX) = sourceImage.at<Vec3b>(i, j);
			currentTargetX++;
		}
	}

	return targetImage;
}
