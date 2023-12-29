#include "FilterParamsValidatorFactory.h"
#include "BrightnessAdjFilterParamsValidator.h"
#include "CropFilterParamsValidator.h"
#include "FlipFilterParamsValidator.h"
#include "ResizeFilterParamsValidator.h"
#include "RotateFilterParamsValidator.h"

FilterParamsValidator* FilterParamsValidatorFactory::GetFilterParamsValidator(const ImageFilterTypesEnum& filterType,
    const vector<float>& filterParams, const cv::Size& imageDimensions)
{
    switch (filterType) {
	case NONE:
		return nullptr;
	case RESIZE:
		return new ResizeFilterParamsValidator(filterParams);
	case ROTATE:
		//TODO can consider taking direction input as string instead of numbers
		return new RotateFilterParamsValidator(filterParams);
	case FLIP:
		//TODO can consider taking direction input as string instead of numbers
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
