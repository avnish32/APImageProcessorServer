#pragma once
#include "ImageFilter.h"

enum RotationDirection {CLOCKWISE, ANTI_CLOCKWISE};

class Rotator : public ImageFilter
{
private:
	RotationDirection _direction;
	u_short _numOfTurns;

	Mat RotateClockwiseOnce();
	Mat RotateAntiClockwiseOnce();
	Mat RotateTwice();

public:
	Rotator();
	Rotator(const Mat& sourceImage, const u_short& numOfTurns, const RotationDirection& rotationMode);
	~Rotator();
	Mat ApplyFilter() override;
};

