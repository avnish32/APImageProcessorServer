#include<opencv2/opencv.hpp>

typedef unsigned short u_short;

using cv::Mat;

#pragma once
class ImageFilter
{
protected:
	Mat _sourceImage;
public:
	ImageFilter();
	ImageFilter(const Mat& sourceImage);
	virtual Mat ApplyFilter() = 0;
	virtual bool CanFilterBeApplied();
};

