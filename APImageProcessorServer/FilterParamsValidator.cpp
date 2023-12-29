#include "FilterParamsValidator.h"

#include<iostream>
#include <string>

FilterParamsValidator::FilterParamsValidator()
{
}

FilterParamsValidator::FilterParamsValidator(const vector<float>& filterParams)
{
	_filterParams = filterParams;
}

FilterParamsValidator::FilterParamsValidator(const vector<float>& filterParams, const cv::Size& imageDimensions)
{
	_filterParams = filterParams;
	_imageDimensions = imageDimensions;
}

bool FilterParamsValidator::ValidateFilterParams()
{
	return true;
}
