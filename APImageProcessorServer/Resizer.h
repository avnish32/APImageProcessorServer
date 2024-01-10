#include "ImageFilter.h"

#pragma once
class Resizer : public ImageFilter
{
private:
	u_short target_width_, target_height_;

public:
	Resizer();
	Resizer(const u_short& targetWidth, const u_short& targetHeight);
	~Resizer();
	Mat ApplyFilter(const Mat& sourceImage) override;
};

