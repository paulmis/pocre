#include "pch.h"

int main()
{
	static std::function<bool(float)> eligible = [](float pix) -> bool { return pix > 0.1; };
	std::map<char, std::vector<cv::Mat>> data;

	bool training = false;
	if (training)
	{
		// Iterate through training images
		for (auto& p : fs::directory_iterator("traindata"))
		{
			// Ascertain the path isn't a textfile path
			if (p.path().extension() == ".txt")
				continue;

			// Load the image
			ocr::page page;
			std::string image_path = p.path().string();
			cv::Mat image = cv::imread(image_path, cv::IMREAD_COLOR);
			if (image.empty())
				EMPTY_IMAGE_C;

			// Delete alpha channel if it exists
			if (image.type() == CV_8UC4)
			{
				cv::Mat tmp_image(image.size(), CV_8UC3);
				cv::cvtColor(image, tmp_image, cv::COLOR_BGRA2GRAY);
				image = tmp_image;
			}

			print_mat_type(image);
			page.set_image(image);

			// Load the text
			std::string text_path = image_path.substr(0, image_path.find_last_of('.')) + ".txt";
			std::ifstream text_in(text_path.c_str());

			if (!text_in.is_open())
				CANNOT_OPEN_FILE_C(text_path);

			std::vector<std::string> text;
			std::string buf;

			while (file_read(text_in, buf))
				text.push_back(buf);
			page.set_text(text);

			// Get training data
			std::cout << image_path << "\n";
			page.set_text(text);
			page.ext_classify(eligible, 50, true);
			std::vector<std::map<char, std::vector<cv::Mat>>> image_data = page.get_training_data();

			// Insert page's training data
			for (int line = 0; line < image_data.size(); line++)
				for (auto& c : image_data[line])
					data[c.first].insert(data[c.first].end(), c.second.begin(), c.second.end());
		}

		ocr::dictionary train(data);
		train.save("dictionary", false);
		journal("Training successful", log_info);
	}

	ocr::dictionary dictionary("dictionary");

	// Get image
	std::string image_name = "0a10";
	std::string path = "traindata/" + image_name + ".png";
	cv::Mat image = cv::imread(path);

	// Create a page
	ocr::page page(image);
	page.ext_classify(eligible, 50);

	// Recognize text and display information
	std::vector<std::string> text = page.get_text_extended(dictionary);
	std::cout << "Recognized text:\n";
	for (std::string& line : text)
		std::cout << line << "\n";
}