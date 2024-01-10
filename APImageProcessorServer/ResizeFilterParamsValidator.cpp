#include "ResizeFilterParamsValidator.h"

#include<iostream>
#include <string>

using std::stoi;

ResizeFilterParamsValidator::ResizeFilterParamsValidator(const vector<float>& filterParams) :FilterParamsValidator(filterParams)
{
}

bool ResizeFilterParamsValidator::ValidateFilterParams()
{
	int targetWidth = filter_params_.at(0);
	int targetHeight = filter_params_.at(1);

	if (targetWidth <= 0 || targetHeight <= 0) {
		msg_logger_->LogError("ERROR: Width and height of resized image cannot be zero or negative.");
		return false;
	}
	return true;
}