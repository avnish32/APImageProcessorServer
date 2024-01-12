#include "ImageFilter.h"

#pragma once
class Resizer : public ImageFilter
{
private:
	u_short target_width_, target_height_;

public:
	Resizer();
	Resizer(const u_short&, const u_short&);
	~Resizer();
	Mat ApplyFilter(const Mat&) override;
};

