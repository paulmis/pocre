#include "pch.h"
#include "sign.h"

namespace ocr
{
	sign::sign(ifstream& _in, Size _heatmap_size)
	{
		if (!load(_in, _heatmap_size))
			journal("Failed to construct a sign", log_error);
	}

	sign::sign(char _c, vector<Mat> _images, Size _heatmap_size)
		: c(_c), images(0)
	{
		make(_images, _heatmap_size);
	}

	// Builds the sign data from the sign's images
	void sign::make(vector<Mat> _images, Size _heatmap_size)
	{
		heatmap = Mat::zeros(_heatmap_size, CV_32FC1);
		for (Mat image : _images)
			add(image);

		for (int y = 0; y < _heatmap_size.height; y++)
			for (int x = 0; x < _heatmap_size.width; x++)
				heatmap.at<float>(y, x) = min((float)1.0000, dec_round(heatmap.at<float>(y, x), 4));
	}

	void sign::save(std::ofstream& _out)
	{
		_out << c << " " << images << " " << hw_factor << "\n";

		for (int y = 0; y < heatmap.size().height; y++)
		{
			for (int x = 0; x < heatmap.size().width; x++)
				_out << fixed << setprecision(5) << heatmap.at<float>(y, x) << " ";

			_out << "\n";
		}
	}

	bool sign::load(std::ifstream& _in, Size _heatmap_size)
	{
		string buf;
		size_t it = 2;

		if (!file_read(_in, buf))
		{
			journal("Missing sign data", log_error);
			return false;
		}

		c = buf[0];
		images = splstr<int>(buf, it);
		hw_factor = splstr<float>(buf, it);
		heatmap = Mat(_heatmap_size, CV_32FC1);

		for (int y = 0; y < heatmap.size().height; y++)
		{
			if (!file_read(_in, buf))
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

	// Builds a single image's data
	void sign::add(Mat _image)
	{
		// Resize the image to fit the heatmap
		float image_weight = (float)images / (float)(images + 1);
		float input_weight = (float)1.0 - image_weight;
		hw_factor = input_weight * _image.size().aspectRatio() + image_weight * hw_factor;
		resize(_image, _image, heatmap.size());

		// Add the shadow
		for (int y = 0; y < heatmap.size().height; y++)
			for (int x = 0; x < heatmap.size().width; x++)
				heatmap.at<float>(y, x) = image_weight * heatmap.at<float>(y, x) +
				input_weight * _image.at<float>(y, x);

		images++;
	}

	result sign::match(Mat _image, Size _original_size, gradient& _gradient)
	{
		// Ascertain that there isn't size mismatch
		if (_image.size() != heatmap.size())
		{
			journal("Can't match sign " + string(c, 1) + ". Image and heatmap are of different sizes.", log_error);
			return DEFAULT_RESULT;
		}

		// Run the image through the gradient
		result result(c);
		for (int y = 0; y < heatmap.size().height; y++)
			for (int x = 0; x < heatmap.size().width; x++)
				result += _gradient.at(_image.at<float>(y, x), heatmap.at<float>(y, x));

		// Normalize the result
		double f = _original_size.aspectRatio() / hw_factor;
		result.val *= min(1.0, ((f > 1.0 ? 1 / f : f) + 0.45) * 0.8);
		return result;
	}

	pair<result, Mat> sign::ext_match(Mat _image, Size _original_size, gradient& _gradient)
	{
		Mat gradient_image(_image.size(), CV_32FC1);

		// Ascertain that there isn't size mismatch
		if (_image.size() != heatmap.size())
		{
			journal("Can't match sign " + std::string(c, 1) + " to the image - size disparity", log_error);
			return make_pair(DEFAULT_RESULT, gradient_image);
		}

		// Run the image through the gradient
		result result(c);
		for (int y = 0; y < heatmap.size().height; y++)
			for (int x = 0; x < heatmap.size().width; x++)
				gradient_image.at<float>(y, x) = _gradient.at(_image.at<float>(y, x), heatmap.at<float>(y, x));

		// Apply the localize kernel
		float localize_data[25] = { 0.0266, 0.0266, 0.04, 0.0266, 0.0266, 0.0266, 0.04, 0.0666, 0.04, 0.0266, 0.04, 0.0666, 0.0933, 0.0666, 0.04, 0.0266, 0.04, 0.0666, 0.04, 0.0266, 0.0266, 0.0266, 0.04, 0.0266, 0.0266 };
		Mat localize_kernel(5, 5, CV_32FC1, localize_data);
		cv::filter2D(gradient_image, gradient_image, -1, localize_kernel);
		
		for (int y = 0; y < heatmap.size().height; y++)
			for (int x = 0; x < heatmap.size().width; x++)
				result += gradient_image.at<float>(y, x);

		// Normalize the result
		result.val /= (long long)heatmap.size().height * heatmap.size().width;
		double f = _original_size.aspectRatio() / hw_factor;
		result.val *= min(1.0, ((f > 1.0 ? 1 / f : f) + 0.45) * 0.8);
		return make_pair(result, gradient_image);
	}

	void sign::show(string _caption)
	{
		imshow(_caption, heatmap);
		waitKey(0);
	}

	char sign::get_char()
	{
		return c;
	}

	Mat sign::get_heatmap()
	{
		return heatmap;
	}
}