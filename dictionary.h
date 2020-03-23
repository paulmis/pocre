#pragma once

namespace ocr
{
	class dictionary
	{
	private:

		cv::Size heatmap_size;
		std::vector<ocr::sign> signs;
		ocr::gradient gradient;

	public:

		dictionary(std::string path);
		dictionary(std::map<char, std::vector<cv::Mat>> _data);
		void show_signs();

		bool save(std::string name, bool save_gradient = true);
		bool load(std::string name);

		ocr::result classify(cv::Mat image);
		std::pair<ocr::result, std::vector<cv::Mat>> ext_classify(cv::Mat image);

		uint size();
		cv::Size image_size();
		std::vector<ocr::sign> get_signs();
	};
}


