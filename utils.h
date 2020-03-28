#pragma once

static const cv::Vec3b red(0, 0, 255), blue(255, 0, 0), green(0, 255, 0), white(255, 255, 255), black(0, 0, 0), yellow(0, 255, 255);
#define ts(x) std::to_string(x)

template<typename T>
T splstr(std::string& str, size_t& it, char delim = ' ')
{
	std::string val;

	while (it < str.size() && str[it] == delim) it++;
	while (it < str.size() && str[it] != delim) val += str[it++];
	it++;

	if constexpr (std::is_same<T, std::string>::value) return val;

	if constexpr (std::is_same<T, int>::value)		 return std::stoi(val);
	if constexpr (std::is_same<T, uint>::value)		 return (uint)std::stoi(val);
	if constexpr (std::is_same<T, long long>::value) return std::stoll(val);
	if constexpr (std::is_same<T, short>::value)     return (short)std::stoi(val);
	if constexpr (std::is_same<T, size_t>::value)	 return (size_t)std::stoi(val);
	if constexpr (std::is_same<T, float>::value)	 return std::stof(val);

	throw;
}

std::string splstr(std::string& str, size_t& it, char delim)
{
	std::string ret;

	while (it < str.size() && str[it] == delim)
		it++;

	while (it < str.size() && str[it] != delim)
		ret += str[it++];

	it++;
	return ret;
}

bool file_read(std::ifstream& in, std::string& str);
void print_mat_type(cv::Mat image, std::string mat_name = "Matrix");
void draw_row(std::vector<cv::Mat> src, cv::Mat dst, cv::Point start, std::function<cv::Vec3b(double)> adapt);
void draw_column(std::vector<cv::Mat> src, cv::Mat dst, cv::Point start, std::function<cv::Vec3b(double)> adapt);
cv::Mat conv3to1(cv::Mat _image);
cv::Mat conv1to3(cv::Mat image);
void blink_image(cv::Mat image, std::string image_name);
void resize(cv::Mat& src, cv::Size size);
void resize(cv::Mat& src, double fx, double fy);
cv::Vec3b one_redgreen(double x);
cv::Vec3b one_white(double x);
float dec_round(float x, int places);
int non_whitespace_count(std::string& str);