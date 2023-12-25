#include "Cropper.h"

#include<iostream>

using std::cout;

using cv::Vec3b;

Cropper::Cropper()
{
	_cropTopLeftCornerX = _cropTopLeftCornerY = 0;
	_targetWidth = _sourceImage.cols;
	_targetHeight = _sourceImage.rows;
}

Cropper::Cropper(const Mat& sourceImage, const u_short& cropTopLeftCornerX, const u_short& cropTopLeftCornerY,
	const u_short& targetWidth, const u_short& targetHeight):ImageFilter(sourceImage)
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

bool Cropper::CanFilterBeApplied()
{
    return _cropTopLeftCornerX <= _sourceImage.cols
        && _cropTopLeftCornerY <= _sourceImage.rows;
}

Mat Cropper::ApplyFilter()
{
	//Calcullate bounds of the source image to process
	int sourceImageColLimit = MIN(_cropTopLeftCornerX + _targetWidth, _sourceImage.cols);
	int sourceImageRowLimit = MIN(_cropTopLeftCornerY + _targetHeight, _sourceImage.rows);
	
	//Initialize target dimensions
	_targetWidth = sourceImageColLimit - _cropTopLeftCornerX;
	_targetHeight = sourceImageRowLimit - _cropTopLeftCornerY;	

	Mat targetImage = Mat(cv::Size(_targetWidth, _targetHeight), _sourceImage.type());

	int currentTargetX = -1, currentTargetY = -1;

	for (int i = _cropTopLeftCornerY; i < sourceImageRowLimit; i++) {
		currentTargetY++;
		currentTargetX = 0;
		for (int j = _cropTopLeftCornerX; j < sourceImageColLimit; j++) {
			targetImage.at<Vec3b>(currentTargetY, currentTargetX) = _sourceImage.at<Vec3b>(i, j);
			currentTargetX++;
		}
	}

	return targetImage;
}
