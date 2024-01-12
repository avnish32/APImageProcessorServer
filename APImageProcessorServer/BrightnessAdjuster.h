#pragma once
#include "ImageFilter.h"
class BrightnessAdjuster : public ImageFilter
{
	//Params reqd in payload: Float brightness adjustment factor from 0 to ...
private:
	float brightness_adj_factor_;

	float ClampPixelValue(float, const float, const float);

public:
	BrightnessAdjuster();
	BrightnessAdjuster(const float&);
	~BrightnessAdjuster();
	Mat ApplyFilter(const Mat&);
};

