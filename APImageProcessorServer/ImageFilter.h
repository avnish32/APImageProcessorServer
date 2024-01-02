#include<opencv2/opencv.hpp>

#include "MsgLogger.h"

typedef unsigned short u_short;

using cv::Mat;

#pragma once

/*
This is an abstract class having a pure virtual function called ApplyFilter.
Its children contain the filter application logic for their respective filters.
*/
class ImageFilter
{
protected:
	MsgLogger* _msgLogger = MsgLogger::GetInstance();
public:
	virtual Mat ApplyFilter(const Mat& sourceImage) = 0;
};