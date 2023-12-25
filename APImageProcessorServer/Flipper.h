#pragma once
#include "ImageFilter.h"
#include "Constants.h"

class Flipper : public ImageFilter
{
	//Params reqd in payload: Flip direction - horizontal/vertical
private:
	FlipDirection _flipDirection;

	Mat _FlipHorizontally();
	Mat _FlipVertically();

public:
	Flipper();
	Flipper(const Mat& sourceImage, const FlipDirection& flipDirection);
	~Flipper();
	Mat ApplyFilter() override;
};

