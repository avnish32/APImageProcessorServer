#include "ImageFilter.h"

#pragma once
class Resizer : public ImageFilter
{
private:
	ushort _targetWidth, _targetHeight;

public:
	Resizer();
	Resizer(const ushort& targetWidth, const ushort& targetHeight);
	~Resizer();
	Mat ApplyFilter(const Mat& sourceImage) override;
};

