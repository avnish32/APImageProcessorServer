#include "ImageFilter.h"

#pragma once
class RGBToGrayscaleConverter : public ImageFilter
{
	//Params reqd: None
public:
	Mat ApplyFilter(const Mat&);
};

