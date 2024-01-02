#include "ImageMetadataProcessor.h"
#include "Constants.h"
#include "FilterParamsValidator.h"
#include "FilterParamsValidatorFactory.h"

#include<iostream>
#include<string>

using std::stoi;
using std::stof;
using std::to_string;

ImageMetadataProcessor::ImageMetadataProcessor(vector<string> imageMetadataStringVector)
{
	_imageMetadataVector = imageMetadataStringVector;
}

ImageMetadataProcessor::~ImageMetadataProcessor()
{
	_msgLogger->LogDebug("ImageMetadataProcessor destroyed.");
}

/*
This function validates the image metadata.
A valid image metadata has the following format: 
"SIZE <image width> <image height> <image size in bytes> <filter type> <filter params...>"
without the quotes.
*/
short ImageMetadataProcessor::ValidateImageMetadata(cv::Size& imageDimensions, uint& imageFileSize,
	ImageFilterTypesEnum& filterTypeEnum, vector<float>& filterParams)
{
	//Sample payload: SIZE 1024 768 2359296 1 800 600
	if (_imageMetadataVector.size() < MIN_IMAGE_METADATA_PARAMS) {
		_msgLogger->LogError("ERROR: Too few parameters in image metadata.");
		return RESPONSE_FAILURE;
	}

	if (_imageMetadataVector.at(0) != SIZE_PAYLOAD_KEY) {
		_msgLogger->LogError("ERROR: Client sent image meta data in wrong format.");
		return RESPONSE_FAILURE;
	}
	try {
		imageDimensions = cv::Size(stoi(_imageMetadataVector.at(1)), stoi(_imageMetadataVector.at(2)));
		imageFileSize = stoi(_imageMetadataVector.at(3));		

		if (_imageMetadataVector.size() > MIN_IMAGE_METADATA_PARAMS) {
			for (int i = 5; i < _imageMetadataVector.size(); i++) {
				filterParams.push_back(stof(_imageMetadataVector.at(i)));
			}
		}
	}
	catch (std::invalid_argument iaExp) {
		_msgLogger->LogError("ERROR: Invalid image size/filter values received.");
		return RESPONSE_FAILURE;
	}

	filterTypeEnum = (ImageFilterTypesEnum)(stoi(_imageMetadataVector.at(4)));
	switch (filterTypeEnum) {
	case BRIGHTNESS_ADJ:
	case CROP:
	case FLIP:
	case RESIZE:
	case ROTATE:
	case RGB_TO_GRAYSCALE:
		break;
	default:
		_msgLogger->LogError("ERROR: Invalid filter type received: "+to_string(filterTypeEnum));
		return RESPONSE_FAILURE;
	}

	FilterParamsValidator* filterParamsValidator = FilterParamsValidatorFactory::GetFilterParamsValidator(filterTypeEnum, 
		filterParams, imageDimensions);

	if (filterParamsValidator == nullptr || !filterParamsValidator->ValidateFilterParams()) {
		_msgLogger->LogError("ERROR: Filter parameter validation failed.");
		delete filterParamsValidator;
		return RESPONSE_FAILURE;
	}

	delete filterParamsValidator;
	return RESPONSE_SUCCESS;
}
