#include<opencv2/opencv.hpp>

typedef unsigned short u_short;

using cv::Mat;

#pragma once
class ImageFilter
{
public:
	virtual Mat ApplyFilter(const Mat& sourceImage) = 0;
};