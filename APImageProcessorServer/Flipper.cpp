#include "Flipper.h"

using cv::Vec3b;
using std::cout;

Mat Flipper::_FlipHorizontally()
{
	//Initialize target image
	Mat targetImage = Mat(cv::Size(_sourceImage.cols, _sourceImage.rows), _sourceImage.type());

	for (int i = 0; i < _sourceImage.rows; i++) {
		for (int j = 0; j < _sourceImage.cols; j++) {
			targetImage.at<Vec3b>(i, j) = _sourceImage.at<Vec3b>(i, _sourceImage.cols - j - 1);
		}
	}

	return targetImage;
}

Mat Flipper::_FlipVertically()
{
	//Initialize target image
	Mat targetImage = Mat(cv::Size(_sourceImage.cols, _sourceImage.rows), _sourceImage.type());

	for (int i = 0; i < _sourceImage.rows; i++) {
		for (int j = 0; j < _sourceImage.cols; j++) {
			targetImage.at<Vec3b>(i, j) = _sourceImage.at<Vec3b>(_sourceImage.rows - i - 1, j);
		}
	}

	return targetImage;
}

Flipper::Flipper()
{
	_flipDirection = HORIZONTAL;
}

Flipper::Flipper(const Mat& sourceImage, const FlipDirection& flipDirection):ImageFilter(sourceImage)
{
	_flipDirection = flipDirection;
}

Flipper::~Flipper()
{
	cout << "\nDestroying Flipper object.";
}

Mat Flipper::ApplyFilter()
{
	switch (_flipDirection) {
	case VERTICAL:
		return _FlipVertically();
	default:
		return _FlipHorizontally();
	}
}
