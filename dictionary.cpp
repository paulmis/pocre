#include "pch.h"
#include "dictionary.h"

namespace ocr
{
	dictionary::dictionary(string path)
	{
		if (!load(path))
			journal("Failed to load a dictionary at " + path, log_error);
	}

	dictionary::dictionary(map<char, vector<Mat>> _data)
	{
		// Calculate the heatmap size
		uint rows = 0, cols = 0, images = 0;

		for (auto& c : _data)
		{
			for (Mat image : c.second)
				rows += image.rows, cols += image.cols;
			images += (uint)c.second.size();

			// Display images
			Size image_size(std::min((int)c.second.size(), 30) * 32 + 1, (1 + (int)floor((c.second.size() - 1) / 30)) * 32 + 1);
			Mat display_image(image_size, CV_32FC1);
			for (int it = 0; it < c.second.size(); it++)
			{
				// Resize the image
				Mat image = c.second[it];
				resize(image, Size(32, 32));

				// Copy the image to its proper position
				int y = (int)floor(it / 30) * 32;
				int x = 32 * (it - (int)floor(it / 30) * 30);
				image.copyTo(display_image(Rect(x, y, image.size().width, image.size().height)));
			}

			blink_image(display_image, string(1, c.first));
		}

		// Make the signs
		heatmap_size = Size(cols / images, rows / images);
		for (auto sign_data : _data)
			signs.push_back(sign(sign_data.first, sign_data.second, heatmap_size));
	}

	void dictionary::show_signs()
	{
		for (sign& s : signs)
			s.show();
	}

	bool dictionary::save(string name, bool save_gradient)
	{
		// Check if the directory exists
		string path = "dictionaries\\" + name + "\\";

		if (!fs::exists(path))
			fs::create_directory(path);

		// Save gradient
		if (save_gradient)
		{
			string gradient_path = path + "gradient.otsf";
			gradient.save(gradient_path);
		}

		// Save signs
		string signs_path = path + "signs.otsf";
		ofstream out(signs_path.c_str());

		if (!out.is_open())
			CANNOT_CREATE_FILE_F(signs_path);

		out << signs.size() << " " << heatmap_size.height << " " << heatmap_size.width << "\n";
		for (size_t it = 0; it < signs.size(); it++)
			signs[it].save(out);

		out.close();
		return true;
	}

	bool dictionary::load(string name)
	{
		// Check if the directory exists
		string path = "dictionaries\\" + name + "\\";

		if (!fs::exists(path))
		{
			journal("Dictionary directory at " + path + " does not exist", log_error);
			return false;
		}

		// Load gradient
		string gradient_path = path + "gradient.otsf";

		if (!gradient.load(gradient_path))
			return false;

		// Load signs
		string signs_path = path + "signs.otsf";
		ifstream in(signs_path.c_str());
		size_t it = 0; string buf;

		if (!in.is_open())		 MISSING_FILE_F(signs_path);
		if (!file_read(in, buf)) MISSING_FILE_DATA_F(signs_path);

		int sings_count = splstr<int>(buf, it);
		heatmap_size = Size(splstr<int>(buf, it), splstr<int>(buf, it));

		for (int s = 0; s < sings_count; s++)
			signs.push_back(sign(in, heatmap_size));

		in.close();
		return true;
	}

	result dictionary::classify(Mat _image, Size _original_size)
	{
		vector<result> results;
		for (sign& sign : signs)
			results.push_back(sign.match(_image, _original_size, gradient));

		return *max_element(results.begin(), results.end(), greater<result>());
	}

	pair<result, vector<Mat>> dictionary::ext_classify(Mat image, Size _original_size)
	{
		vector<Mat> images;
		vector<result> results;

		for (sign& s : signs)
		{
			pair<result, Mat> res = s.ext_match(image, _original_size, gradient);
			results.push_back(res.first);
			images.push_back(res.second);
		}

		sort(results.begin(), results.end(), greater<ocr::result>());
		for (int it = 0; it < results.size() && it < 5; it++)
			cout << results[it];
		cout << "\n";

		return make_pair(results.front(), images);
	}

	uint dictionary::size()
	{
		return (uint)signs.size();
	}

	Size dictionary::image_size()
	{
		return heatmap_size;
	}

	vector<sign> dictionary::get_signs()
	{
		return signs;
	}
}
