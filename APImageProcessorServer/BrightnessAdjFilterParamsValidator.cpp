#include "BrightnessAdjFilterParamsValidator.h"

#include<iostream>
#include<string>

BrightnessAdjFilterParamsValidator::BrightnessAdjFilterParamsValidator(const vector<float>& filter_params) :FilterParamsValidator(filter_params)
{
}

bool BrightnessAdjFilterParamsValidator::ValidateFilterParams()
{
	float brightness_adj_factor = filter_params_.at(0);
	if (brightness_adj_factor < 0) {
		msg_logger_->LogError("ERROR: Invalid value for BRIGHTNESS ADJUSTMENT filter parameters.");
		return false;
	}
	return true;
}
