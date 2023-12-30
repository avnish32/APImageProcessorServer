#include<opencv2/opencv.hpp>

#include "MsgLogger.h"

typedef unsigned short u_short;

using cv::Mat;

#pragma once
class ImageFilter
{
protected:
	MsgLogger* _msgLogger = MsgLogger::GetInstance();
public:
	virtual Mat ApplyFilter(const Mat& sourceImage) = 0;
};