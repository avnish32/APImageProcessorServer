#pragma once
#include "ImageConstructor.h"

/*
This is a concrete class of the abstract class ImageConstructor.
It constructs a double channel image.
*/
class TwoChannelImageConstructor : public ImageConstructor
{
public:
	TwoChannelImageConstructor(const map<unsigned short, string>& imageDataMap, const Size& imageDimensions);
	~TwoChannelImageConstructor();
	Mat ConstructImage() override;
};

