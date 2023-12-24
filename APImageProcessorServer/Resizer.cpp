#include "Resizer.h"

//#include<math.h>

using cv::Vec3b;

Resizer::Resizer()
{
	_targetWidth = _targetHeight = 1;
}

Resizer::Resizer(const Mat& sourceImage, const u_short& targetWidth, const u_short& targetHeight):ImageFilter(sourceImage)
{
	_targetWidth = targetWidth;
	_targetHeight = targetHeight;
}

Mat Resizer::ApplyFilter()
{
	//Params reqd in payload: targetWidth, targetHeight
	//Nearest-neighbor algorithm used for resizing. Reference: https://courses.cs.vt.edu/~masc1044/L17-Rotation/ScalingNN.html
	u_short sourceWidth = _sourceImage.cols;
	u_short sourceHeight = _sourceImage.rows;

	if (sourceWidth == _targetWidth && sourceHeight == _targetHeight) {
		//TODO return the original image
	}

	Mat targetImage = Mat(cv::Size(_targetWidth, _targetHeight), _sourceImage.type());
	for (int i = 0; i < _targetHeight; i++) {
		for (int j = 0; j < _targetWidth; j++) {
			int sourceImageRow = round(((float)i / _targetHeight) * sourceHeight);
			int sourceImageCol = round(((float)j / _targetWidth) * sourceWidth);
			targetImage.at<Vec3b>(i, j) = _sourceImage.at<Vec3b>(sourceImageRow, sourceImageCol);
		}
	}

	return targetImage;
}
