#pragma once
#include "FilterParamsValidator.h"

class CropFilterParamsValidator : public FilterParamsValidator
{
private:
	bool IsCoordinateOutsideImage(short x, short y);
public:
	CropFilterParamsValidator(const vector<float>& filterParams, Size imageDimensions);
	bool ValidateFilterParams();
};