#pragma once
#include "ImageFilter.h"
class Cropper : public ImageFilter
{
	//Params reqd in payload: Co-ordinates of the top left corner of the cropped image, width, height
private:
	u_short _cropTopLeftCornerX;
	u_short _cropTopLeftCornerY;
	u_short _targetWidth;
	u_short _targetHeight;

public:
	Cropper();
	Cropper(const Mat& sourceImage, const u_short& cropTopLeftCornerX, const u_short& cropTopLeftCornerY,
		const u_short& targetWidth, const u_short& targetHeight);
	~Cropper();
	bool CanFilterBeApplied();
	Mat ApplyFilter() override;
};

