#include "BrightnessAdjFilterParamsValidator.h"

#include<iostream>
#include<string>

BrightnessAdjFilterParamsValidator::BrightnessAdjFilterParamsValidator(const vector<float>& filterParams) :FilterParamsValidator(filterParams)
{
}

bool BrightnessAdjFilterParamsValidator::ValidateFilterParams()
{
	float brightnessAdjFactor = _filterParams.at(0);
	if (brightnessAdjFactor < 0) {
		_msgLogger->LogError("ERROR: Invalid value for BRIGHTNESS ADJUSTMENT filter parameters.");
		return false;
	}
	return true;
}
