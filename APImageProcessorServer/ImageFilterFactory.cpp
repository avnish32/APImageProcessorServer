#include "ImageFilterFactory.h"
#include "ImageFilter.h"
#include "BrightnessAdjuster.h"
#include "Cropper.h"
#include "Flipper.h"
#include "Resizer.h"
#include "Rotator.h"
#include "RGBToGrayscaleConverter.h"

#include<iostream>

ImageFilter* ImageFilterFactory::GetImageFilter(const ImageFilterTypesEnum& filterType, const std::vector<float>& filterParams)
{
    std::cout << "\nInside image filter factory. Filter type: " << filterType;

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
