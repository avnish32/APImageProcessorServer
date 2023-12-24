#include "ImageFilter.h"

#pragma once
class Resizer : public ImageFilter
{
private:
	unsigned short _targetWidth, _targetHeight;

public:
	Resizer();
	Resizer(const Mat& sourceImage, const u_short& targetWidth, const u_short& targetHeight);
	Mat ApplyFilter() override;
};

