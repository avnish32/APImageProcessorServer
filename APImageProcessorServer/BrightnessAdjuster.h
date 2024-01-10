#pragma once
#include "ImageFilter.h"
class BrightnessAdjuster : public ImageFilter
{
	//Params reqd in payload: Float brightness adjustment factor from 0 to ...
private:
	float brightness_adj_factor_;

	float ClampPixelValue(float unClampedValue, const float minValue, const float maxValue);

public:
	BrightnessAdjuster();
	BrightnessAdjuster(const float& brightnessAdjFactor);
	~BrightnessAdjuster();
	Mat ApplyFilter(const Mat& sourceImage);
};

