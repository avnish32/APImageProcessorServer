#include "FilterParamsValidator.h"
#include "Constants.h"

#include<opencv2/opencv.hpp>
#include<vector>

#pragma once
class FilterParamsValidatorFactory
{
public:
	static FilterParamsValidator* GetFilterParamsValidator(const ImageFilterTypesEnum& filterType, 
		const vector<float>& filterParams, const cv::Size& imageDimensions);
};

