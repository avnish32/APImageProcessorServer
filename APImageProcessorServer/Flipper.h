#pragma once
#include "ImageFilter.h"
#include "Constants.h"

class Flipper : public ImageFilter
{
	//Params reqd in payload: Flip direction - horizontal/vertical
private:
	FlipDirection _flipDirection;

	Mat _FlipHorizontally(const Mat& sourceImage);
	Mat _FlipVertically(const Mat& sourceImage);

public:
	Flipper();
	Flipper(const FlipDirection& flipDirection);
	~Flipper();
	Mat ApplyFilter(const Mat& sourceImage) override;
};

