#include "ImageProcessor.h"
#include "ImageFilter.h"
#include "ImageFilterFactory.h"
#include "ImageConstructorFactory.h"

#include<iostream>
#include<string>

using cv::imwrite;
using cv::imshow;

using std::to_string;

ImageProcessor::ImageProcessor()
{
	msg_logger_->LogDebug("Image processor default constructor.");
	image_ = Mat(1, 1, CV_8UC1);
}

ImageProcessor::ImageProcessor(Mat image)
{
	image_ = image;
}

/*
Parameterized constructor; constructs an image having imageDimensions from the given imageDataMap.
*/
ImageProcessor::ImageProcessor(map<unsigned short, std::string> image_data_map, const Size& image_dimensions, const uint& image_file_size)
{
	short num_of_channels = image_file_size / (image_dimensions.width * image_dimensions.height);
	
	msg_logger_->LogDebug("Inside ImageProcessor. Number of channels: " + to_string(num_of_channels));

	ImageConstructor* image_constructor = ImageConstructorFactory::GetImageConstructor(num_of_channels, image_data_map, image_dimensions);
	if (image_constructor == nullptr) {
		msg_logger_->LogError("ERROR: Constructing image with " + to_string(num_of_channels) + "is currently not supported.");
		return;
	}
	
	image_ = image_constructor->ConstructImage();
}

ImageProcessor::~ImageProcessor()
{
	msg_logger_->LogDebug("Image processor destructor.");
}

void ImageProcessor::DisplayImage(cv::String window_name)
{
	cv::namedWindow(window_name, cv::WINDOW_KEEPRATIO);
	imshow(window_name, image_);

	cv::waitKey(0);
	cv::destroyWindow(window_name);
}

/*
This function gets the appropriate image filter class depending on the filter type and then returns
the resultant image after applying the filter.
*/
Mat ImageProcessor::ApplyFilter(ImageFilterTypesEnum filter_type, vector<float> filter_params)
{
	ImageFilter* image_filter = ImageFilterFactory::GetImageFilter(filter_type, filter_params);
	return image_filter->ApplyFilter(image_);
}

void ImageProcessor::SaveImage(cv::String save_address)
{
	bool was_image_written = imwrite(save_address, image_);
	if (!was_image_written) {
		msg_logger_->LogError("ERROR: Image could not be written to file.");
		return;
	}
	msg_logger_->LogDebug("Image written to file successfully.");
}

void ImageProcessor::SaveImage(Mat image_to_save, cv::String save_address)
{
	bool was_image_written = imwrite(save_address, image_to_save);
	if (!was_image_written) {
		msg_logger_->LogError("ERROR: Image could not be written to file.");
		return;
	}
	msg_logger_->LogDebug("Image written to file successfully.");
}

Mat ImageProcessor::GetImage()
{
	return image_;
}