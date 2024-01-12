#include "ResizeFilterParamsValidator.h"

#include<iostream>
#include <string>

using std::stoi;

ResizeFilterParamsValidator::ResizeFilterParamsValidator(const vector<float>& filter_params) :FilterParamsValidator(filter_params)
{
}

bool ResizeFilterParamsValidator::ValidateFilterParams()
{
	int target_width = filter_params_.at(0);
	int target_height = filter_params_.at(1);

	if (target_width <= 0 || target_height <= 0) {
		msg_logger_->LogError("ERROR: Width and height of resized image cannot be zero or negative.");
		return false;
	}
	return true;
}