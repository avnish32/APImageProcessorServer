#include "BrightnessAdjFilterParamsValidator.h"

#include<iostream>
#include<string>

BrightnessAdjFilterParamsValidator::BrightnessAdjFilterParamsValidator(const vector<float>& filterParams) :FilterParamsValidator(filterParams)
{
}

bool BrightnessAdjFilterParamsValidator::ValidateFilterParams()
{
	float brightnessAdjFactor = filter_params_.at(0);
	if (brightnessAdjFactor < 0) {
		msg_logger_->LogError("ERROR: Invalid value for BRIGHTNESS ADJUSTMENT filter parameters.");
		return false;
	}
	return true;
}
