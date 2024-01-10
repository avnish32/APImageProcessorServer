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
	short cropTopLeftCornerX = filter_params_.at(0);
	short cropTopLeftCornerY = filter_params_.at(1);
	short targetWidth = filter_params_.at(2);
	short targetHeight = filter_params_.at(3);

	if (targetWidth <= 0 || targetHeight <= 0) {
		msg_logger_->LogError("ERROR: Invalid values for CROP filter parameters.");
		return false;
	}

	if (IsCoordinateOutsideImage(cropTopLeftCornerX, cropTopLeftCornerY)) {
		msg_logger_->LogError("ERROR: Given coordinate lies outside the image.");
		return false;
	}

	return true;
}

bool CropFilterParamsValidator::IsCoordinateOutsideImage(short x, short y)
{
	return x < 0 || y < 0 || x > image_dimensions_.width || y > image_dimensions_.height;
}