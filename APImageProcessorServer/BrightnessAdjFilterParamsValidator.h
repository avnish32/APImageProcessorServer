#pragma once
#include "FilterParamsValidator.h"
class BrightnessAdjFilterParamsValidator : public FilterParamsValidator
{
public:
    BrightnessAdjFilterParamsValidator(const vector<float>&);
    bool ValidateFilterParams();
};