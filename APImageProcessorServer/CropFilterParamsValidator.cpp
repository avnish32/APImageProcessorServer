#include "CropFilterParamsValidator.h"

#include<iostream>
#include<string>

using std::stoi;

CropFilterParamsValidator::CropFilterParamsValidator(const vector<float>& filterParams, Size imageDimensions) 
	:FilterParamsValidator(filterParams, imageDimensions)
{

}

bool CropFilterParamsValidator::ValidateFilterParams()
{
	short cropTopLeftCornerX = _filterParams.at(0);
	short cropTopLeftCornerY = _filterParams.at(1);
	short targetWidth = _filterParams.at(2);
	short targetHeight = _filterParams.at(3);

	if (targetWidth <= 0 || targetHeight <= 0) {
		_msgLogger->LogError("ERROR: Invalid values for CROP filter parameters.");
		return false;
	}

	if (_IsCoordinateOutsideImage(cropTopLeftCornerX, cropTopLeftCornerY)) {
		_msgLogger->LogError("ERROR: Given coordinate lies outside the image.");
		return false;
	}

	return true;
}

bool CropFilterParamsValidator::_IsCoordinateOutsideImage(short x, short y)
{
	return x < 0 || y < 0 || x > _imageDimensions.width || y > _imageDimensions.height;
}