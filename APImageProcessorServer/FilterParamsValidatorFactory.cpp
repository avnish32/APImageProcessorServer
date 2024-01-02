#include "FilterParamsValidatorFactory.h"
#include "BrightnessAdjFilterParamsValidator.h"
#include "CropFilterParamsValidator.h"
#include "FlipFilterParamsValidator.h"
#include "ResizeFilterParamsValidator.h"
#include "RotateFilterParamsValidator.h"

/*
This function checks filterType and instantiates an object of the corresponding concrete derived class of FilterParamsValidator
with filterParams and imageDimensions.
*/
FilterParamsValidator* FilterParamsValidatorFactory::GetFilterParamsValidator(const ImageFilterTypesEnum& filterType,
    const vector<float>& filterParams, const cv::Size& imageDimensions)
{
    switch (filterType) {
	case NONE:
		return nullptr;
	case RESIZE:
		return new ResizeFilterParamsValidator(filterParams);
	case ROTATE:
		return new RotateFilterParamsValidator(filterParams);
	case FLIP:
		return new FlipFilterParamsValidator(filterParams);
	case CROP:
		return new CropFilterParamsValidator(filterParams, imageDimensions);
	case RGB_TO_GRAYSCALE:
		return new FilterParamsValidator();
	case BRIGHTNESS_ADJ:
		return new BrightnessAdjFilterParamsValidator(filterParams);
	default:
		return nullptr;
    }
    return nullptr;
}
