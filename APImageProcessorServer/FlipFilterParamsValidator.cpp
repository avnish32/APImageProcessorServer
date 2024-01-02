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
	short direction = _filterParams.at(0);
	FlipDirection flipDirection = (FlipDirection)direction;

	switch (flipDirection) {
	case HORIZONTAL:
	case VERTICAL:
		break;
	default:
		_msgLogger->LogError("ERROR: Invalid direction given for flipping.");
		return false;
	}
	return true;
}
