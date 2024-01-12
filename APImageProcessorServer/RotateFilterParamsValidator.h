#pragma once
#include "FilterParamsValidator.h"
class RotateFilterParamsValidator : public FilterParamsValidator
{
public:
	RotateFilterParamsValidator(const vector<float>&);
	bool ValidateFilterParams();
};