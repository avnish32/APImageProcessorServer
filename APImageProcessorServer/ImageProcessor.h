#include<opencv2/opencv.hpp>
#include<map>
#include<vector>

#include "Constants.h"
#include "MsgLogger.h"

#pragma once

using cv::Mat;
using cv::Size;

using std::map;
using std::vector;

/*
This class handles various tasks related to the processing of image.
*/
class ImageProcessor
{
private:
	Mat image_;
	MsgLogger* msg_logger_ = MsgLogger::GetInstance();

public:
	ImageProcessor();
	ImageProcessor(Mat);
	ImageProcessor(map<unsigned short, std::string>, const Size&, const uint&);
	~ImageProcessor();

	void DisplayImage(cv::String);

	Mat ApplyFilter(ImageFilterTypesEnum, vector<float>);

	void SaveImage(cv::String);
	void SaveImage(Mat, cv::String);

	Mat GetImage();
};

