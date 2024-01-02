#pragma once
#include "ImageConstructor.h"

/*
This is a concrete class of the abstract class ImageConstructor.
It constructs a triple channel image.
*/
class ThreeChannelImageConstructor :public ImageConstructor
{
public:
	ThreeChannelImageConstructor(const map<unsigned short, string>& imageDataMap, const Size& imageDimensions);
	~ThreeChannelImageConstructor();
	Mat ConstructImage() override;
};

