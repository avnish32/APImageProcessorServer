#include "FlipFilterParamsValidator.h"
#include "Constants.h"

#include<iostream>
#include<string>

using std::stoi;

FlipFilterParamsValidator::FlipFilterParamsValidator(const vector<float>& filter_params) :FilterParamsValidator(filter_params)
{
}

bool FlipFilterParamsValidator::ValidateFilterParams()
{
	short direction = filter_params_.at(0);
	FlipDirection flip_direction = (FlipDirection)direction;

	switch (flip_direction) {
	case HORIZONTAL:
	case VERTICAL:
		break;
	default:
		msg_logger_->LogError("ERROR: Invalid direction given for flipping.");
		return false;
	}
	return true;
}
