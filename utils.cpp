#include "pch.h"
#include "utils.h"

bool file_read(std::ifstream& in, std::string& str)
{
	char buf[8192];
	in.getline(buf, 8192);
	std::string tmp = buf;

	if (tmp.size() > 0)
	{
		str = tmp;
		return true;
	}

	return false;
}


void print_mat_type(cv::Mat image, std::string mat_name)
{
	std::string r;
	int type = image.type();

	uchar depth = type & CV_MAT_DEPTH_MASK;
	uchar chans = 1 + (type >> CV_CN_SHIFT);

	switch (depth) {
	case CV_8U:  r = "8U"; break;
	case CV_8S:  r = "8S"; break;
	case CV_16U: r = "16U"; break;
	case CV_16S: r = "16S"; break;
	case CV_32S: r = "32S"; break;
	case CV_32F: r = "32F"; break;
	case CV_64F: r = "64F"; break;
	default:     r = "User"; break;
	}

	r += "C";
	r += (chans + '0');

	jinf(mat_name + " is a " + r + " of size " + std::to_string(image.cols) + "x" + std::to_string(image.rows));
}

void draw_row(std::vector<cv::Mat> src, cv::Mat dst, cv::Point start, std::function<cv::Vec3b(double)> adapt)
{
	for (cv::Mat image : src)
	{
		cv::Mat adapted_image(image.size(), CV_8UC3);

		for (int y = 0; y < image.size().height; y++)
			for (int x = 0; x < image.size().width; x++)
				adapted_image.at<cv::Vec3b>(y, x) = adapt(image.at<float>(y, x));

		adapted_image.copyTo(dst(cv::Rect(start.x, start.y, image.cols, image.rows)));
		start.x += image.cols;
	}
}

void draw_column(std::vector<cv::Mat> src, cv::Mat dst, cv::Point start, std::function<cv::Vec3b(double)> adapt)
{
	for (cv::Mat image : src)
	{
		cv::Mat adapted_image(image.size(), CV_8UC3);

		for (int y = 0; y < image.size().height; y++)
			for (int x = 0; x < image.size().width; x++)
				adapted_image.at<cv::Vec3b>(y, x) = adapt(image.at<float>(y, x));

		adapted_image.copyTo(dst(cv::Rect(start.x, start.y, image.cols, image.rows)));
		start.y += image.rows;
	}
}

// Convert CV_8UCX to CV_32FC1
cv::Mat conv3to1(cv::Mat _image)
{
	if (_image.channels() == 1)
		return _image;

	cv::Mat image(_image.size(), CV_32FC1);
	for (int y = 0; y < _image.size().height; y++)
		for (int x = 0; x < _image.size().width; x++)
		{
			cv::Vec3b px = _image.at<cv::Vec3b>(y, x);
			image.at<float>(y, x) = 0.0013072 * (px[0] + px[1] + px[2]);
		}

	return image;
}

cv::Mat conv1to3(cv::Mat image)
{
	cv::Mat new_image(image.size(), CV_8UC3);

	for (int y = 0; y < image.size().height; y++)
		for (int x = 0; x < image.size().width; x++)
			for (int c = 0; c < 3; c++)
				new_image.at<cv::Vec3b>(y, x)[c] = (uchar)(255.0 * image.at<float>(y, x));

	return new_image;
}

std::string get_date(std::string format)
{
	time_t now = time(0);
	tm timeinfo;
	localtime_s(&timeinfo, &now);
	char buf[256];
	strftime(buf, 256, format.c_str(), &timeinfo);
	return buf;
}

void blink_image(cv::Mat image, std::string image_name = "image")
{
	cv::imshow(image_name, image);
	cv::waitKey();
	cv::destroyWindow(image_name);
}

void resize(cv::Mat& src, cv::Size size)
{
	cv::resize(src, src, size, 0.0, 0.0, src.size().area() < size.area() ? cv::INTER_CUBIC : cv::INTER_AREA);
}

void resize(cv::Mat& src,  double fx, double fy)
{
	resize(src, src, cv::Size(fx * src.size().width, fy * src.size().height));
}

cv::Vec3b one_redgreen(double x)
{
	if (x > 0.5) return cv::Vec3b(0, 255, (int)(255 - (510 * (x - 0.5))));
	else		 return cv::Vec3b(0, (int)(255 - (510 * (0.5 - x))), 255);
}

cv::Vec3b one_white(double x)
{
	return cv::Vec3b((int)round(x * 255), (int)round(x * 255), (int)round(x * 255));
}

float dec_round(float x, int places)
{
	int mul = (int)pow(10, places);
	return ((int)((float)x * (float)mul)) / (float)mul;
}