#pragma once
#include "ImageFilter.h"
class BrightnessAdjuster : public ImageFilter
{
	//Params reqd in payload: Float brightness adjustment factor from 0 to ...
private:
	float _brightnessAdjFactor;

	float _clampPixelValue(float unClampedValue, const float minValue, const float maxValue);
public:
	BrightnessAdjuster();
	BrightnessAdjuster(const Mat& sourceImage, const float& brightnessAdjFactor);
	~BrightnessAdjuster();
	Mat ApplyFilter();
};

