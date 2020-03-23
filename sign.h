#pragma once

namespace ocr
{
	using namespace std;

	class sign
	{
		char c;
		size_t images;
		cv::Mat heatmap;

	public:

		sign();
		sign(std::ifstream& in, cv::Size _heatmap_size);
		sign(char _c, vector<cv::Mat> _images, cv::Size _heatmap_size);

		void make(vector<cv::Mat> _images, cv::Size _heatmap_size);
		ocr::result match(cv::Mat _image, ocr::gradient& gradient);
		std::pair<ocr::result, cv::Mat> ext_match(cv::Mat _image, ocr::gradient& gradient);
		void add(cv::Mat _image);
		void show(std::string _caption = "");

		void save(std::ofstream& out);
		bool load(std::ifstream& in, cv::Size _heatmap_size);

		char get_char();
		cv::Mat get_heatmap();
	};

	struct sign_comparator
	{
		bool operator()(sign& l, sign& r)
		{
			return l.get_char() < r.get_char();
		}
	};
}