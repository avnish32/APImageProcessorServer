#pragma once
#include "ImageFilter.h"
#include "Constants.h"

class Rotator : public ImageFilter
{
private:
	RotationDirection _direction;
	u_short _numOfTurns;

	Mat _RotateClockwiseOnce();
	Mat _RotateAntiClockwiseOnce();
	Mat _RotateTwice();

public:
	Rotator();
	Rotator(const Mat& sourceImage, const u_short& numOfTurns, const RotationDirection& rotationMode);
	~Rotator();
	Mat ApplyFilter() override;
};

