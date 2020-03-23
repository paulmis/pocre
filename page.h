#pragma once

namespace ocr
{
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

		void classify(function<bool(float)> eligible, int min_points);
		void ext_classify(function<bool(float)> eligible, size_t _min_blob, bool draw_text = false, std::string display_text = "extended");
		vector<string> get_text(ocr::dictionary& dictionary);
		vector<string> get_text_extended(ocr::dictionary& dictionary);
		vector<map<char, vector<cv::Mat>>> get_training_data();

		optional<cv::Mat> draw(bool draw_letters = true);
	};
}