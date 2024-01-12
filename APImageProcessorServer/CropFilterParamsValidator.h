#pragma once
#include "FilterParamsValidator.h"

class CropFilterParamsValidator : public FilterParamsValidator
{
private:
	bool IsCoordinateOutsideImage(short, short);
public:
	CropFilterParamsValidator(const vector<float>&, Size);
	bool ValidateFilterParams();
};