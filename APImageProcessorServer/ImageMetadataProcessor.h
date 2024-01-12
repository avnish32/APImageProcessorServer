#include<opencv2/opencv.hpp>
#include<vector>
#include<string>

#include "Constants.h"
#include "MsgLogger.h"

using std::vector;
using std::string;

#pragma once

/*
This class deals with various operations on information about the image,
such as its dimensions, size and the filter to be applied on it.
*/
class ImageMetadataProcessor
{
private:
	vector<string> image_metadata_vector_;
	MsgLogger* msg_logger_ = MsgLogger::GetInstance();

public:
	ImageMetadataProcessor(vector<string>);
	~ImageMetadataProcessor();
	short ValidateImageMetadata(cv::Size&, uint&,  ImageFilterTypesEnum&,
		vector<float>&);

};

