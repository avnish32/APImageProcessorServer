#include "BrightnessAdjFilterParamsValidator.h"

#include<iostream>
#include<string>

using std::cout;

BrightnessAdjFilterParamsValidator::BrightnessAdjFilterParamsValidator(const vector<float>& filterParams) :FilterParamsValidator(filterParams)
{
}

bool BrightnessAdjFilterParamsValidator::ValidateFilterParams()
{
	float brightnessAdjFactor = _filterParams.at(0);
	if (brightnessAdjFactor < 0) {
		cout << "\nERROR: Invalid value for BRIGHTNESS ADJUSTMENT filter parameters.";
		return false;
	}
	return true;
}
