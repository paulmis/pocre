#pragma once

namespace ocr
{
	// A wrapper for text images
	class page
	{
	private:

		Mat image;
		vector<line> lines;
		vector<string> text;

		bool check_text();

	public:

		page();
		page(cv::Mat _image);

		void set_text(string _text);
		void set_text(vector<string> _text);
		void set_image(cv::Mat _image);

		void classify(function<bool(float)> _eligible, int _min_points);
		void ext_classify(function<bool(float)> _eligible, size_t _min_blob, bool _draw_letters = false);
		vector<string> get_text(ocr::dictionary& _dictionary);
		vector<string> get_text_extended(ocr::dictionary& _dictionary);
		vector<map<char, vector<Mat>>> get_training_data();

		Mat draw(bool _draw_letters = true);
	};
}