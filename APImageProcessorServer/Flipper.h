#pragma once
#include "ImageFilter.h"
#include "Constants.h"

class Flipper : public ImageFilter
{
	//Params reqd in payload: Flip direction - horizontal/vertical
private:
	FlipDirection flip_direction_;

	Mat FlipHorizontally(const Mat&);
	Mat FlipVertically(const Mat&);

public:
	Flipper();
	Flipper(const FlipDirection&);
	~Flipper();
	Mat ApplyFilter(const Mat&) override;
};

