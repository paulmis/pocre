#include "pch.h"
#include "dictionary.h"

ocr::dictionary::dictionary(std::string path)
{
	if (!load(path))
		journal("Failed to load a dictionary at " + path, log_error);
}

ocr::dictionary::dictionary(std::map<char, std::vector<cv::Mat>> _data)
{
	// Calculate the heatmap size
	uint rows = 0, cols = 0, images = 0;

	for (auto& c : _data)
	{
		for (cv::Mat image : c.second)
			rows += image.rows, cols += image.cols;
		images += (uint)c.second.size();

		// Display images
		cv::Size image_size(std::min((int)c.second.size(), 30) * 32 + 1, (1 + (int)floor((c.second.size() - 1) / 30)) * 32 + 1);
		cv::Mat display_image(image_size, CV_32FC1);
		for (int it = 0; it < c.second.size(); it++)
		{
			// Resize the image
			Mat image = c.second[it];
			resize(image, Size(32, 32));
			
			// Copy the image to its proper position
			int y = (int)floor(it / 30) * 32;
			int x = 32 * (it - (int)floor(it / 30) * 30);
			image.copyTo(display_image(cv::Rect(x, y, image.size().width, image.size().height)));
		}

		blink_image(display_image, std::string(1, c.first));
	}

	heatmap_size = cv::Size(cols / images, rows / images);

	// Make the signs
	for (auto sign_data : _data)
		signs.push_back(ocr::sign(sign_data.first, sign_data.second, heatmap_size));
}

void ocr::dictionary::show_signs()
{
	for (sign& s : signs)
		s.show();
}

bool ocr::dictionary::save(std::string name, bool save_gradient)
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
	{
		journal("Cannot save dictionary to " + signs_path + " - the directory doesn't exist", log_error);
		return false;
	}

	out << signs.size() << " " << heatmap_size.height << " " << heatmap_size.width << "\n";

	for (size_t it = 0; it < signs.size(); it++)
		signs[it].save(out);

	out.close();
	return true;
}

bool ocr::dictionary::load(std::string name)
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

	if (!in.is_open())
	{
		journal("Missing dictionary files at " + signs_path, log_error);
		return false;
	}

	std::string buf;

	if (!file_read(in, buf))
	{
		journal("Missing dictionary data", log_error);
		return false;
	}

	size_t it = 0;
	uint s = splstr<uint>(buf, it);
	heatmap_size = cv::Size(splstr<int>(buf, it), splstr<int>(buf, it));

	for (uint sign = 0; sign < s; sign++)
		signs.push_back(ocr::sign(in, heatmap_size));

	in.close();
	return true;
}

ocr::result ocr::dictionary::classify(cv::Mat image)
{
	vector<ocr::result> results;
	for (ocr::sign& sign : signs)
		results.push_back(sign.match(image, gradient));

	return *max_element(results.begin(), results.end(), greater<ocr::result>());
}

std::pair<ocr::result, std::vector<cv::Mat>> ocr::dictionary::ext_classify(cv::Mat image)
{
	vector<Mat> images;
	vector<ocr::result> results;

	for (ocr::sign& sign : signs)
	{
		pair<result, Mat> res = sign.ext_match(image, gradient);
		results.push_back(res.first);
		images.push_back(res.second);
	}

	sort(results.begin(), results.end(), greater<ocr::result>());
	for (int it = 0; it < results.size() && it < 5; it++)
		cout << results[it];
	cout << "\n";

	return make_pair(results.front(), images);
}

uint ocr::dictionary::size()
{
	return (uint)signs.size();
}

cv::Size ocr::dictionary::image_size()
{
	return heatmap_size;
}

std::vector<ocr::sign> ocr::dictionary::get_signs()
{
	return signs;
}
