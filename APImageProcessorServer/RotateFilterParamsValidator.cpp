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
	short direction = _filterParams.at(0);
	short numOfTurns = _filterParams.at(1);

	RotationDirection rotationDirection = (RotationDirection)direction;

	switch (rotationDirection) {
	case CLOCKWISE:
	case ANTI_CLOCKWISE:
		break;
	default:
		_msgLogger->LogError("ERROR: Invalid direction given for rotation: "+to_string(rotationDirection));
		return false;
	}

	if (numOfTurns < 0) {
		_msgLogger->LogError("ERROR: Invalid number of turns given for rotation: "+to_string(numOfTurns));
		return false;
	}

	return true;
}