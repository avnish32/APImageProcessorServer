#pragma once
#include "ImageFilter.h"
#include "Constants.h"

class Rotator : public ImageFilter
{
	//Params reqd in payload: Rotation direction, number of turns
private:
	RotationDirection direction_;
	u_short num_of_turns_;

	Mat RotateClockwiseOnce(const Mat& sourceImage);
	Mat RotateAntiClockwiseOnce(const Mat& sourceImage);
	Mat RotateTwice(const Mat& sourceImage);

public:
	Rotator();
	Rotator(const RotationDirection& rotationMode, const u_short& numOfTurns);
	~Rotator();
	Mat ApplyFilter(const Mat& sourceImage) override;
};

