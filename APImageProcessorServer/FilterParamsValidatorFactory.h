#include "FilterParamsValidator.h"
#include "Constants.h"

#include<opencv2/opencv.hpp>
#include<vector>

#pragma once

/*
This is the factory class for objects of the FilterParamsValidator class.
It instantiates a FilterParamsValidator object depending on the type of the filter it receives.
*/
class FilterParamsValidatorFactory
{
public:
	static FilterParamsValidator* GetFilterParamsValidator(const ImageFilterTypesEnum& filterType, 
		const vector<float>& filterParams, const cv::Size& imageDimensions);
};

