#include<vector>
#include<opencv2/opencv.hpp>

#include "MsgLogger.h"

#pragma once

using std::vector;
using cv::Size;

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