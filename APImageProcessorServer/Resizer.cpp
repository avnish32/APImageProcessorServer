#include "Resizer.h"

#include<iostream>

using cv::Vec3b;
using std::cout;

Resizer::Resizer()
{
	_targetWidth = _targetHeight = 1;
}

Resizer::Resizer(const u_short& targetWidth, const u_short& targetHeight)
{
	_targetWidth = targetWidth;
	_targetHeight = targetHeight;
}

Resizer::~Resizer()
{
	cout << "\nDestroying Resizer.";
}

Mat Resizer::ApplyFilter(const Mat& sourceImage)
{
	cout << "\nApplying resize filter. Target width: "<<_targetWidth<<" | Target height: "<<_targetHeight;

	//Params reqd in payload: targetWidth, targetHeight
	//Nearest-neighbor algorithm used for resizing. Reference: https://courses.cs.vt.edu/~masc1044/L17-Rotation/ScalingNN.html
	u_short sourceWidth = sourceImage.cols;
	u_short sourceHeight = sourceImage.rows;

	if (sourceWidth == _targetWidth && sourceHeight == _targetHeight) {
		//TODO return the original image
	}

	Mat targetImage = Mat(cv::Size(_targetWidth, _targetHeight), sourceImage.type());
	for (int i = 0; i < _targetHeight; i++) {
		for (int j = 0; j < _targetWidth; j++) {
			int sourceImageRow = round(((float)i / _targetHeight) * sourceHeight);
			int sourceImageCol = round(((float)j / _targetWidth) * sourceWidth);
			targetImage.at<Vec3b>(i, j) = sourceImage.at<Vec3b>(sourceImageRow, sourceImageCol);
		}
	}

	return targetImage;
}
