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
FilterParamsValidator* FilterParamsValidatorFactory::GetFilterParamsValidator(const ImageFilterTypesEnum& filter_type,
    const vector<float>& filter_params, const cv::Size& image_dimensions)
{
    switch (filter_type) {
	case NONE:
		return nullptr;
	case RESIZE:
		return new ResizeFilterParamsValidator(filter_params);
	case ROTATE:
		return new RotateFilterParamsValidator(filter_params);
	case FLIP:
		return new FlipFilterParamsValidator(filter_params);
	case CROP:
		return new CropFilterParamsValidator(filter_params, image_dimensions);
	case RGB_TO_GRAYSCALE:
		return new FilterParamsValidator();
	case BRIGHTNESS_ADJ:
		return new BrightnessAdjFilterParamsValidator(filter_params);
	default:
		return nullptr;
    }
    return nullptr;
}
