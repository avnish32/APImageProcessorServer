#include "RotateFilterParamsValidator.h"
#include "Constants.h"

#include<iostream>
#include<string>

using std::stoi;
using std::to_string;

RotateFilterParamsValidator::RotateFilterParamsValidator(const vector<float>& filterParams) :FilterParamsValidator(filterParams)
{
}

bool RotateFilterParamsValidator::ValidateFilterParams()
{
	short direction = filter_params_.at(0);
	short numOfTurns = filter_params_.at(1);

	RotationDirection rotationDirection = (RotationDirection)direction;

	switch (rotationDirection) {
	case CLOCKWISE:
	case ANTI_CLOCKWISE:
		break;
	default:
		msg_logger_->LogError("ERROR: Invalid direction given for rotation: "+to_string(rotationDirection));
		return false;
	}

	if (numOfTurns < 0) {
		msg_logger_->LogError("ERROR: Invalid number of turns given for rotation: "+to_string(numOfTurns));
		return false;
	}

	return true;
}