#pragma once
#include "ImageFilter.h"
class Cropper : public ImageFilter
{
	//Params reqd in payload: Co-ordinates of the top left corner of the cropped image, width, height
private:
	u_short top_left_corner_x_;
	u_short top_left_corner_y;
	u_short target_width_;
	u_short target_height_;

public:
	Cropper();
	Cropper(const u_short& cropTopLeftCornerX, const u_short& cropTopLeftCornerY,
		const u_short& targetWidth, const u_short& targetHeight);
	~Cropper();
	Mat ApplyFilter(const Mat& sourceImage) override;
};

