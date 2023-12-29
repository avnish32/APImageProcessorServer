#include "Cropper.h"

#include<iostream>

using std::cout;

using cv::Vec3b;

Cropper::Cropper()
{
	_cropTopLeftCornerX = _cropTopLeftCornerY = 0;
	_targetWidth = 1;
	_targetHeight = 1;
}

Cropper::Cropper(const u_short& cropTopLeftCornerX, const u_short& cropTopLeftCornerY,
	const u_short& targetWidth, const u_short& targetHeight)
{
	_cropTopLeftCornerX = cropTopLeftCornerX;
	_cropTopLeftCornerY = cropTopLeftCornerY;
	_targetWidth = targetWidth;
	_targetHeight = targetHeight;
}

Cropper::~Cropper()
{
	cout << "\nCropper object destroyed.";
}

Mat Cropper::ApplyFilter(const Mat& sourceImage)
{
	cout << "\nApplying crop filter Top left corner Coordinates: ("<<_cropTopLeftCornerX<<","<<_cropTopLeftCornerY
		<< ") | Target width: " << _targetWidth << " | Target height: " << _targetHeight;;

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
