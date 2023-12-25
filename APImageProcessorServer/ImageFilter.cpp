#include "ImageFilter.h"

ImageFilter::ImageFilter()
{
	_sourceImage = Mat(1, 1, CV_8UC1);
}

ImageFilter::ImageFilter(const Mat& sourceImage)
{
	_sourceImage = sourceImage;
}

bool ImageFilter::CanFilterBeApplied()
{
	return true;
}
