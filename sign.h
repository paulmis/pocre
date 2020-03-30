#pragma once

namespace ocr
{
	using namespace std;

	class sign
	{
		char c;
		int images;
		float hw_factor;
		Mat heatmap;

	public:

		sign();
		sign(ifstream& _in, Size _heatmap_size);
		sign(char _c, vector<Mat> _images, Size _heatmap_size);

		void save(ofstream& _out);
		bool load(ifstream& _in, Size _heatmap_size);

		void make(vector<Mat> _images, Size _heatmap_size);
		void add(Mat _image);

		result match(Mat _image, Size _original_size, gradient& _gradient);
		pair<result, Mat> ext_match(Mat _image, Size _original_size, gradient& _gradient);
		void show(string _caption = "");

		char get_char();
		Mat get_heatmap();
	};

	struct sign_comparator
	{
		bool operator()(sign& l, sign& r)
		{
			return l.get_char() < r.get_char();
		}
	};
}