#pragma once

namespace ocr
{
	class dictionary
	{
	private:

		Size heatmap_size;
		vector<sign> signs;
		gradient gradient;

	public:

		dictionary(string path);
		dictionary(map<char, vector<Mat>> _data);
		void show_signs();

		bool save(string name, bool save_gradient = true);
		bool load(string name);

		result classify(Mat _image, Size _original_size);
		pair<result, vector<Mat>> ext_classify(Mat image, Size _original_size);

		uint size();
		Size image_size();
		vector<sign> get_signs();
	};
}


