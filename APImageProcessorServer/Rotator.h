#pragma once
#include "ImageFilter.h"
#include "Constants.h"

class Rotator : public ImageFilter
{
	//Params reqd in payload: Rotation direction, number of turns
private:
	RotationDirection _direction;
	u_short _numOfTurns;

	Mat _RotateClockwiseOnce(const Mat& sourceImage);
	Mat _RotateAntiClockwiseOnce(const Mat& sourceImage);
	Mat _RotateTwice(const Mat& sourceImage);

public:
	Rotator();
	Rotator(const RotationDirection& rotationMode, const u_short& numOfTurns);
	~Rotator();
	Mat ApplyFilter(const Mat& sourceImage) override;
};

