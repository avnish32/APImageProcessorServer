#include "Resizer.h"

#include<opencv2/opencv.hpp>

#include<iostream>
#include<string>

using cv::Vec3b;

using std::cout;
using std::to_string;

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
	//cout << "\nDestroying Resizer.";
	_msgLogger->LogDebug("Destroying Resizer.");
}

Mat Resizer::ApplyFilter(const Mat& sourceImage)
{
	//cout << "\nApplying resize filter. Target width: "<<_targetWidth<<" | Target height: "<<_targetHeight;
	_msgLogger->LogError("Resizing image. Target width: " + to_string(_targetWidth) 
		+ " | Target height: " + to_string(_targetHeight));

	//Params reqd in payload: targetWidth, targetHeight
	//Nearest-neighbor algorithm used for resizing. Reference: https://courses.cs.vt.edu/~masc1044/L17-Rotation/ScalingNN.html
	u_short sourceWidth = sourceImage.cols;
	u_short sourceHeight = sourceImage.rows;

	if (sourceWidth == _targetWidth && sourceHeight == _targetHeight) {
		return sourceImage;
	}

	_msgLogger->LogDebug("Resizer::source image type: "+ to_string(sourceImage.type()) + " | 8UC3: " + to_string(CV_8UC3));
	
	Mat targetImage = Mat(cv::Size(_targetWidth, _targetHeight), sourceImage.type());
	for (int i = 0; i < _targetHeight; i++) {
		for (int j = 0; j < _targetWidth; j++) {
			//TODO remove after testing
			//_msgLogger->LogDebug("i = " + to_string(i) + " | j= " + to_string(j));
			int sourceImageRow = round(((float)i / _targetHeight) * sourceHeight);
			int sourceImageCol = round(((float)j / _targetWidth) * sourceWidth);

			sourceImageRow = MIN(sourceImageRow, sourceHeight - 1);
			sourceImageCol = MIN(sourceImageCol, sourceWidth - 1);

			targetImage.at<Vec3b>(i, j) = sourceImage.at<Vec3b>(sourceImageRow, sourceImageCol);
		}
	}

	return targetImage;
}
