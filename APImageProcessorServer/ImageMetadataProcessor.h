#include<opencv2/opencv.hpp>
#include<vector>
#include<string>

#include "Constants.h"

using std::vector;
using std::string;

#pragma once

class ImageMetadataProcessor
{
private:
	vector<string> _imageMetadataVector;

public:
	ImageMetadataProcessor(vector<string>);
	~ImageMetadataProcessor();
	short ValidateImageMetadata(cv::Size& imageDimensions, uint& imageFileSize,  ImageFilterTypesEnum& filterTypeEnum,
		vector<float>& filterParams);

};

