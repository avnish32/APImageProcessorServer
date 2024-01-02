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
	vector<string> _imageMetadataVector;
	MsgLogger* _msgLogger = MsgLogger::GetInstance();

public:
	ImageMetadataProcessor(vector<string>);
	~ImageMetadataProcessor();
	short ValidateImageMetadata(cv::Size& imageDimensions, uint& imageFileSize,  ImageFilterTypesEnum& filterTypeEnum,
		vector<float>& filterParams);

};

