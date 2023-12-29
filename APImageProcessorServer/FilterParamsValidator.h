#include<vector>
#include<opencv2/opencv.hpp>

#pragma once

using std::vector;
using cv::Size;

class FilterParamsValidator
{
protected:
	vector<float> _filterParams;
	Size _imageDimensions;

public:
	FilterParamsValidator();
	FilterParamsValidator(const vector<float>& filterParams);
	FilterParamsValidator(const vector<float>& filterParams, const Size& imageDimensions);
	virtual bool ValidateFilterParams();
};