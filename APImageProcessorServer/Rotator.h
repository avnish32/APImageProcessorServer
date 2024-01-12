#pragma once
#include "ImageFilter.h"
#include "Constants.h"

class Rotator : public ImageFilter
{
	//Params reqd in payload: Rotation direction, number of turns
private:
	RotationDirection direction_;
	u_short num_of_turns_;

	Mat RotateClockwiseOnce(const Mat&);
	Mat RotateAntiClockwiseOnce(const Mat&);
	Mat RotateTwice(const Mat&);

public:
	Rotator();
	Rotator(const RotationDirection&, const u_short&);
	~Rotator();
	Mat ApplyFilter(const Mat&) override;
};

