#include "RotateFilterParamsValidator.h"
#include "Constants.h"

#include<iostream>
#include<string>

using std::stoi;
using std::to_string;

RotateFilterParamsValidator::RotateFilterParamsValidator(const vector<float>& filter_params) :FilterParamsValidator(filter_params)
{
}

bool RotateFilterParamsValidator::ValidateFilterParams()
{
	short direction = filter_params_.at(0);
	short no_of_turns = filter_params_.at(1);

	RotationDirection rotation_direction = (RotationDirection)direction;

	switch (rotation_direction) {
	case CLOCKWISE:
	case ANTI_CLOCKWISE:
		break;
	default:
		msg_logger_->LogError("ERROR: Invalid direction given for rotation: "+to_string(rotation_direction));
		return false;
	}

	if (no_of_turns < 0) {
		msg_logger_->LogError("ERROR: Invalid number of turns given for rotation: "+to_string(no_of_turns));
		return false;
	}

	return true;
}