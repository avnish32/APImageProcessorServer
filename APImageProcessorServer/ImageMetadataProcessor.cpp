#include "ImageMetadataProcessor.h"
#include "Constants.h"
#include "FilterParamsValidator.h"
#include "FilterParamsValidatorFactory.h"

#include<iostream>

using std::cout;
using std::stoi;
using std::stof;

ImageMetadataProcessor::ImageMetadataProcessor(vector<string> imageMetadataStringVector)
{
	_imageMetadataVector = imageMetadataStringVector;
}

ImageMetadataProcessor::~ImageMetadataProcessor()
{
	cout << "\nImageMetadataProcessor destroyed.";
}

short ImageMetadataProcessor::ValidateImageMetadata(cv::Size& imageDimensions, uint& imageFileSize,
	ImageFilterTypesEnum& filterTypeEnum, vector<float>& filterParams)
{
	//Sample payload: SIZE 1024 768 2359296 1 800 600
	if (_imageMetadataVector.size() < MIN_IMAGE_METADATA_PARAMS) {
		cout << "\nERROR: Too few parameters in image metadata.";
		return RESPONSE_FAILURE;
	}

	if (_imageMetadataVector.at(0) != SIZE_PAYLOAD_KEY) {
		cout << "\nClient sent image meta data in wrong format.";
		return RESPONSE_FAILURE;
	}
	try {
		imageDimensions = cv::Size(stoi(_imageMetadataVector.at(1)), stoi(_imageMetadataVector.at(2)));
		imageFileSize = stoi(_imageMetadataVector.at(3));
		//TODO check which exception is raised if enum is not found from int value
		filterTypeEnum = (ImageFilterTypesEnum)(stoi(_imageMetadataVector.at(4)));

		if (_imageMetadataVector.size() > MIN_IMAGE_METADATA_PARAMS) {
			for (int i = 5; i < _imageMetadataVector.size(); i++) {
				filterParams.push_back(stof(_imageMetadataVector.at(i)));
			}
		}
	}
	catch (std::invalid_argument iaExp) {
		cout << "\nInvalid image size/filter values received.";
		return RESPONSE_FAILURE;
	}

	FilterParamsValidator* filterParamsValidator = FilterParamsValidatorFactory::GetFilterParamsValidator(filterTypeEnum, 
		filterParams, imageDimensions);

	if (filterParamsValidator == nullptr || !filterParamsValidator->ValidateFilterParams()) {
		cout << "\nFilter parameter validation failed.";
		delete filterParamsValidator;
		return RESPONSE_FAILURE;
	}

	delete filterParamsValidator;
	return RESPONSE_SUCCESS;
}
