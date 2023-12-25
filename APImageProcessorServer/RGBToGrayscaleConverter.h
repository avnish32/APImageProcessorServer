#include "ImageFilter.h"

#pragma once
class RGBToGrayscaleConverter : public ImageFilter
{
	//Params reqd: None
public:
	RGBToGrayscaleConverter();
	RGBToGrayscaleConverter(Mat sourceImage);
	~RGBToGrayscaleConverter();
	Mat ApplyFilter();
};

