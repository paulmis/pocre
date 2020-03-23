#include "pch.h"
#include "sign.h"

namespace ocr
{
	sign::sign(std::ifstream& in, cv::Size _heatmap_size)
	{
		if (!load(in, _heatmap_size))
			journal("Failed to construct a sign", log_error);
	}

	sign::sign(char _c, vector<Mat> _images, cv::Size _heatmap_size)
		: c(_c), images(0)
	{
		make(_images, _heatmap_size);
	}

	void sign::make(vector<Mat> _images, cv::Size _heatmap_size)
	{
		heatmap = Mat::zeros(_heatmap_size, CV_32FC1);

		for (Mat image : _images)
			add(image);

		for (int y = 0; y < _heatmap_size.height; y++)
			for (int x = 0; x < _heatmap_size.width; x++)
				heatmap.at<float>(y, x) = min((float)1.0000, dec_round(heatmap.at<float>(y, x), 4));
	}

	result sign::match(cv::Mat _image, ocr::gradient& gradient)
	{
		// Ascertain that there isn't size mismatch
		if (_image.size() != heatmap.size())
		{
			journal("Can't match sign " + std::string(c, 1) + " to the image - size disparity", log_error);
			return DEFAULT_RESULT;
		}

		ocr::result result(c);

		// Run the image through the gradient
		for (int y = 0; y < heatmap.size().height; y++)
			for (int x = 0; x < heatmap.size().width; x++)
				result += gradient.at(_image.at<float>(y, x), heatmap.at<float>(y, x));

		// Normalize the result
		result.val /= (long long)heatmap.size().height * heatmap.size().width;

		return result;
	}

	std::pair<ocr::result, cv::Mat> sign::ext_match(cv::Mat _image, ocr::gradient& gradient)
	{
		cv::Mat gradient_image(_image.size(), CV_32FC1);

		// Ascertain that there isn't size mismatch
		if (_image.size() != heatmap.size())
		{
			journal("Can't match sign " + std::string(c, 1) + " to the image - size disparity", log_error);
			return make_pair(DEFAULT_RESULT, gradient_image);
		}

		ocr::result result(c);

		// Run the image through the gradient
		for (int y = 0; y < heatmap.size().height; y++)
		{
			for (int x = 0; x < heatmap.size().width; x++)
			{
				gradient_image.at<float>(y, x) = gradient.at(_image.at<float>(y, x), heatmap.at<float>(y, x));
				result += gradient_image.at<float>(y, x);
			}
		}

		// Normalize the result
		result.val /= (long long)heatmap.size().height * heatmap.size().width;

		return make_pair(result, gradient_image);
	}

	void sign::add(Mat _image)
	{
		// Resize the image to fit the heatmap
		resize(_image, _image, heatmap.size());
		float image_weight = (float)images / (float)(images + 1);
		float input_weight = (float)1.0 - image_weight;

		// Add the shadow
		for (int y = 0; y < heatmap.size().height; y++)
			for (int x = 0; x < heatmap.size().width; x++)
				heatmap.at<float>(y, x) = image_weight * heatmap.at<float>(y, x) +
					input_weight * _image.at<float>(y, x);

		images++;
	}

	void sign::show(string caption)
	{
		imshow(caption, heatmap);
		waitKey(0);
	}

	void sign::save(std::ofstream& out)
	{
		out << c << " " << images << "\n";

		for (int y = 0; y < heatmap.size().height; y++)
		{
			for (int x = 0; x < heatmap.size().width; x++)
				out << fixed << setprecision(5) << heatmap.at<float>(y, x) << " ";

			out << "\n";
		}
	}

	bool sign::load(std::ifstream& in, cv::Size _heatmap_size)
	{
		std::string buf;
		size_t it = 2;

		if (!file_read(in, buf))
		{
			journal("Missing sign data", log_error);
			return false;
		}

		c = buf[0];
		images = splstr<int>(buf, it);
		heatmap = cv::Mat(_heatmap_size, CV_32FC1);

		for (int y = 0; y < heatmap.size().height; y++)
		{
			if (!file_read(in, buf))
			{
				journal("Missing sign data", log_error);
				return false;
			}

			it = 0;

			for (int x = 0; x < heatmap.size().width; x++)
				heatmap.at<float>(y, x) = splstr<float>(buf, it);
		}

		return true;
	}

	char sign::get_char()
	{
		return c;
	}

	cv::Mat sign::get_heatmap()
	{
		return heatmap;
	}
}