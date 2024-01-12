#include "FilterParamsValidator.h"

#include<iostream>
#include <string>

FilterParamsValidator::FilterParamsValidator()
{
}

FilterParamsValidator::FilterParamsValidator(const vector<float>& filter_params)
{
	filter_params_ = filter_params;
}

FilterParamsValidator::FilterParamsValidator(const vector<float>& filter_params, const cv::Size& image_dimensions)
{
	filter_params_ = filter_params;
	image_dimensions_ = image_dimensions;
}

bool FilterParamsValidator::ValidateFilterParams()
{
	return true;
}
