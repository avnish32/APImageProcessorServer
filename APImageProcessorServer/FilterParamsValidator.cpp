#include "FilterParamsValidator.h"

#include<iostream>
#include <string>

FilterParamsValidator::FilterParamsValidator()
{
}

FilterParamsValidator::FilterParamsValidator(const vector<float>& filterParams)
{
	filter_params_ = filterParams;
}

FilterParamsValidator::FilterParamsValidator(const vector<float>& filterParams, const cv::Size& imageDimensions)
{
	filter_params_ = filterParams;
	image_dimensions_ = imageDimensions;
}

bool FilterParamsValidator::ValidateFilterParams()
{
	return true;
}
