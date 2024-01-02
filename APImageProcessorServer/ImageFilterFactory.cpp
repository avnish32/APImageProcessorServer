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
ImageFilter* ImageFilterFactory::GetImageFilter(const ImageFilterTypesEnum& filterType, const std::vector<float>& filterParams)
{
    MsgLogger* _msgLogger = MsgLogger::GetInstance();
    _msgLogger->LogDebug("Inside image filter factory. Filter type: " + std::to_string(filterType));

    switch (filterType) {
    case NONE:
        return nullptr;
    case BRIGHTNESS_ADJ:
        return new BrightnessAdjuster(filterParams.at(0));
    case CROP:
        return new Cropper(filterParams.at(0), filterParams.at(1), filterParams.at(2), filterParams.at(3));
    case FLIP:
        return new Flipper((FlipDirection)(filterParams.at(0)));
    case RESIZE:
        return new Resizer(filterParams.at(0), filterParams.at(1));
    case ROTATE:
        return new Rotator((RotationDirection)filterParams.at(0), filterParams.at(1));
    case RGB_TO_GRAYSCALE:
        return new RGBToGrayscaleConverter();
    default:
        return nullptr;
    }
    return nullptr;
}
