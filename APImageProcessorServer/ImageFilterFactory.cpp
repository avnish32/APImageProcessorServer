#include "ImageFilterFactory.h"
#include "ImageFilter.h"
#include "BrightnessAdjuster.h"
#include "Cropper.h"
#include "Flipper.h"
#include "Resizer.h"
#include "Rotator.h"
#include "RGBToGrayscaleConverter.h"

#include<string>

/*
This function checks filterType and instantiates an object of the corresponding concrete derived class of ImageFilter
with filterParams.
*/
ImageFilter* ImageFilterFactory::GetImageFilter(const ImageFilterTypesEnum& filter_type, const std::vector<float>& filter_params)
{
    MsgLogger* _msgLogger = MsgLogger::GetInstance();
    _msgLogger->LogDebug("Inside image filter factory. Filter type: " + std::to_string(filter_type));

    switch (filter_type) {
    case NONE:
        return nullptr;
    case BRIGHTNESS_ADJ:
        return new BrightnessAdjuster(filter_params.at(0));
    case CROP:
        return new Cropper(filter_params.at(0), filter_params.at(1), filter_params.at(2), filter_params.at(3));
    case FLIP:
        return new Flipper((FlipDirection)(filter_params.at(0)));
    case RESIZE:
        return new Resizer(filter_params.at(0), filter_params.at(1));
    case ROTATE:
        return new Rotator((RotationDirection)filter_params.at(0), filter_params.at(1));
    case RGB_TO_GRAYSCALE:
        return new RGBToGrayscaleConverter();
    default:
        return nullptr;
    }
    return nullptr;
}
