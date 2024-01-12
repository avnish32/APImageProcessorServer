#include "CropFilterParamsValidator.h"

#include<iostream>
#include<string>

using std::stoi;

CropFilterParamsValidator::CropFilterParamsValidator(const vector<float>& filter_params, Size image_dimensions) 
	:FilterParamsValidator(filter_params, image_dimensions)
{

}

bool CropFilterParamsValidator::ValidateFilterParams()
{
	short top_left_corner_x = filter_params_.at(0);
	short top_left_corner_y = filter_params_.at(1);
	short target_width = filter_params_.at(2);
	short target_height = filter_params_.at(3);

	if (target_width <= 0 || target_height <= 0) {
		msg_logger_->LogError("ERROR: Invalid values for CROP filter parameters.");
		return false;
	}

	if (IsCoordinateOutsideImage(top_left_corner_x, top_left_corner_y)) {
		msg_logger_->LogError("ERROR: Given coordinate lies outside the image.");
		return false;
	}

	return true;
}

bool CropFilterParamsValidator::IsCoordinateOutsideImage(short x, short y)
{
	return x < 0 || y < 0 || x > image_dimensions_.width || y > image_dimensions_.height;
}