#pragma once
#include "ImageFilter.h"
class Cropper : public ImageFilter
{
	//Params reqd in payload: Co-ordinates of the top left corner of the cropped image, width, height
private:
	ushort _cropTopLeftCornerX;
	ushort _cropTopLeftCornerY;
	ushort _targetWidth;
	ushort _targetHeight;

public:
	Cropper();
	Cropper(const ushort& cropTopLeftCornerX, const ushort& cropTopLeftCornerY,
		const ushort& targetWidth, const ushort& targetHeight);
	~Cropper();
	Mat ApplyFilter(const Mat& sourceImage) override;
};

