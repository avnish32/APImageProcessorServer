#include "FlipFilterParamsValidator.h"
#include "Constants.h"

#include<iostream>
#include<string>

using std::stoi;

FlipFilterParamsValidator::FlipFilterParamsValidator(const vector<float>& filterParams) :FilterParamsValidator(filterParams)
{
}

bool FlipFilterParamsValidator::ValidateFilterParams()
{
	short direction = filter_params_.at(0);
	FlipDirection flipDirection = (FlipDirection)direction;

	switch (flipDirection) {
	case HORIZONTAL:
	case VERTICAL:
		break;
	default:
		msg_logger_->LogError("ERROR: Invalid direction given for flipping.");
		return false;
	}
	return true;
}
