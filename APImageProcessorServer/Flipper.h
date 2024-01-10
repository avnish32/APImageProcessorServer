#pragma once
#include "ImageFilter.h"
#include "Constants.h"

class Flipper : public ImageFilter
{
	//Params reqd in payload: Flip direction - horizontal/vertical
private:
	FlipDirection flip_direction_;

	Mat FlipHorizontally(const Mat& sourceImage);
	Mat FlipVertically(const Mat& sourceImage);

public:
	Flipper();
	Flipper(const FlipDirection& flipDirection);
	~Flipper();
	Mat ApplyFilter(const Mat& sourceImage) override;
};

