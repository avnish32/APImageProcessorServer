#include<vector>
#include<opencv2/opencv.hpp>

#include "MsgLogger.h"

#pragma once

using std::vector;
using cv::Size;

/*
This is an abstract class having a virtrual function ValidateFilterParams().
The children of this class contain validation logic for filter parameters of
each filter that the application supports.
*/
class FilterParamsValidator
{
protected:
	vector<float> _filterParams;
	Size _imageDimensions;
	MsgLogger* _msgLogger = MsgLogger::GetInstance();

public:
	FilterParamsValidator();
	FilterParamsValidator(const vector<float>& filterParams);
	FilterParamsValidator(const vector<float>& filterParams, const Size& imageDimensions);
	virtual bool ValidateFilterParams();
};