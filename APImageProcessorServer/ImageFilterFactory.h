#include<opencv2/opencv.hpp>
#include<vector>

#include "ImageFilter.h"
#include "Constants.h"
#include "MsgLogger.h"

#pragma once

/*
This is the factory class for objects of the ImageFilter class.
It instantiates an ImageFilter object depending on the type of the filter it receives.
*/
class ImageFilterFactory
{
public:
	static ImageFilter* GetImageFilter(const ImageFilterTypesEnum& filterType,
		const std::vector<float>& filterParams);
};