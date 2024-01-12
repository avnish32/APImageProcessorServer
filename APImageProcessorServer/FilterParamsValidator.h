#include<vector>
#include<opencv2/opencv.hpp>

#include "MsgLogger.h"

#pragma once

using std::vector;
using cv::Size;

/*
This is an abstract class having a virtual function ValidateFilterParams().
The children of this class contain validation logic for filter parameters of
each filter that the application supports.
*/
class FilterParamsValidator
{
protected:
	vector<float> filter_params_;
	Size image_dimensions_;
	MsgLogger* msg_logger_ = MsgLogger::GetInstance();

public:
	FilterParamsValidator();
	FilterParamsValidator(const vector<float>&);
	FilterParamsValidator(const vector<float>&, const Size&);
	virtual bool ValidateFilterParams();
};