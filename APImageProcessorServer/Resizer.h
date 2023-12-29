#include "ImageFilter.h"

#pragma once
class Resizer : public ImageFilter
{
private:
	unsigned short _targetWidth, _targetHeight;

public:
	Resizer();
	Resizer(const u_short& targetWidth, const u_short& targetHeight);
	~Resizer();
	Mat ApplyFilter(const Mat& sourceImage) override;
};

