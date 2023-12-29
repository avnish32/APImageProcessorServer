#include<opencv2/opencv.hpp>
#include<vector>

#include "ImageFilter.h"
#include "Constants.h"

#pragma once

class ImageFilterFactory
{
public:
	static ImageFilter* GetImageFilter(const ImageFilterTypesEnum& filterType,
		const std::vector<float>& filterParams);
};