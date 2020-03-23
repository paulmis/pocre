#include "pch.h"
#include "page.h"

namespace ocr
{
	bool page::check_text()
	{
		// Check if the number of lines match
		if (lines.size() != text.size())
		{
			jerr("Missmatch in the number of lines: expected " + std::to_string(lines.size()) + " lines, but the text is " + std::to_string(text.size()) + " lines long");
			return false;
		}

		// Check if the number of blobs in each line match
		for (int l = 0; l < lines.size(); l++)
		{
			if (lines[l].size() != text[l].size())
			{
				jerr("Missmatch in line " + std::to_string(l + 1) + ": expected " + std::to_string(lines[l].size()) + " signs but the text is " + std::to_string(text[l].size()) + " characters long");
				return false;
			}
		}

		return true;
	}

	page::page()
	{}

	page::page(Mat _image)
	{
		set_image(_image);
	}

	void page::set_text(std::string _text)
	{
		// Transform string to vector of strings by \n delim
		size_t it = 0;

		while (it < _text.size())
		{
			string str;
			while (it < _text.size() && _text[it++] != '\n') str += _text[it];
			if (str.size() > 0)								 text.push_back(str);
		}
	}

	void page::set_text(vector<string> _text)
	{
		for (std::string& line : _text)
		{
			line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
			line.erase(std::remove(line.begin(), line.end(), '.'), line.end());
			line.erase(std::remove(line.begin(), line.end(), ','), line.end());
			line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
			line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
		}

		text = _text;
	}

	// Set the image on which to perform recognition
	void page::set_image(Mat _image)
	{
		// Reset variables
		lines.clear();
		text.clear();

		// Preprocessing
		cv::imshow("0", _image);
		if (_image.channels() > 1)
		{
			for (int y = 0; y < _image.rows; y++)
				for (int x = 0; x < _image.cols; x++)
				{
					cv::Vec3b px = _image.at<Vec3b>(y, x);
					int mx = min(255, px[0] + 15);
					_image.at<Vec3b>(y, x) = (px[0] == px[1] && px[1] == px[2] ? cv::Vec3b(mx, mx, mx) : cv::Vec3b(0, 0, 0));
				}

			cv::imshow("1", _image);
			_image = conv3to1(_image);
		}

		static vector<Point> mov = { Point(-1, 0), Point(0, -1), Point(1, 0), Point(0, 1),
									 Point(-1, -1), Point(1, -1), Point(-1, 1), Point(1, 1) };

		cv::Rect frame(cv::Point(), _image.size());
		vector<vector<bool>> visited(_image.rows, vector<bool>(_image.cols, false));
		for (int y = 0; y < _image.rows; y++)
			for (int x = 0; x < _image.cols; x++)
				if (!visited[y][x])
				{
					queue<cv::Point> queue;
					vector<cv::Point> points;
					queue.push({ x, y });
					visited[y][x] = true;

					while (!queue.empty())
					{
						cv::Point cur = queue.front(); queue.pop();
						points.push_back(cur);

						for (cv::Point mv : mov)
						{
							cv::Point nxt(cur.x + mv.x, cur.y + mv.y);
							if (frame.contains(nxt) && !visited[nxt.y][nxt.x] && fabs(_image.at<float>(cur) - _image.at<float>(nxt)) < 0.02)
							{
								visited[nxt.y][nxt.x] = true;
								queue.push(nxt);
							}
						}
					}

					if (points.size() > 50)
						for (cv::Point pt : points)
							_image.at<float>(pt) = 0.0;
				}

		cv::imshow("2", _image);
		cv::threshold(_image, _image, 0.5, 1.0, cv::THRESH_TOZERO);
		cv::imshow("3", _image);
		cv::GaussianBlur(_image, _image, cv::Size(5, 5), 0.4);
		cv::imshow("4", _image);
		cv::resize(_image, _image, cv::Size(), 3.0, 3.0, INTER_CUBIC);
		cv::imshow("5", _image);
		cv::threshold(_image, _image, 0.4, 1.0, cv::THRESH_TOZERO);
		cv::imshow("6", _image);

		for (int y = 0; y < _image.rows; y++)
			for (int x = 0; x < _image.cols; x++)
				_image.at<float>(y, x) = min(static_cast<float>(1.0), _image.at<float>(y, x));

		image = _image;
	}

	// Calassify all eligible blobs on the image
	// Uses a BFS where the condition check is given by the _eligible function
	void page::classify(function<bool(float)> eligible, int min_points)
	{
		static vector<Point> mov = { Point(-1, 0), Point(0, -1), Point(1, 0), Point(0, 1),
									 Point(-1, -1), Point(1, -1), Point(-1, 1), Point(1, 1) };

		// Declare containers
		Rect frame(cv::Point(), image.size());
		std::vector<blob> blobs;
		vector<vector<bool>> visited(image.rows, vector<bool>(image.cols, false));

		// Find signs
		for (int y = 0; y < image.size().height; y++)
		{
			for (int x = 0; x < image.size().width; x++)
			{
				// Check eligibility of the point
				if (!visited[y][x] && eligible(image.at<float>(y, x)))
				{
					ocr::blob blob;
					vector<cv::Point> points = blob.make(cv::Point(x, y), image, visited, eligible, frame);

					// Insert to blob if it's big enough
					if (points.size() > min_points&& points.size() < min_points * 15)
					{
						// Insert the blob into a line
						blob.buffer(image.size());
						bool inserted = false;

						for (line& l : lines)
							if (l.inside_vertically(blob.y_mid()))
							{
								bool inside_other_blob = false;
								for (ocr::blob& b : l.get_blobs())
									if (blob.inside(b.mid()))
									{
										inside_other_blob = true;
										for (cv::Point pt : points)
											image.at<float>(pt.y, pt.x) = 0.0;
									}

								if (!inside_other_blob)
									l.insert(blob);
								inserted = true;
							}

						// Create a new line if insertion failed
						if (!inserted)
						{
							lines.push_back(line());
							lines.back().insert(blob);
						}
					}

					// Otherwise erase all its points from the image
					else
						for (cv::Point pt : points)
							image.at<float>(pt.y, pt.x) = 0.0;
				}
			}
		}

		// Sort the lines
		sort(lines.begin(), lines.end(), [](line& l, line& r) -> bool
			{
				return l.height() < r.height();
			});
	}

	void page::ext_classify(function<bool(float)> eligible, size_t _min_blob, bool draw_text, std::string display_text)
	{
		classify(eligible, _min_blob);
		optional<Mat> image = draw(draw_text);

		if (!image.has_value())
		{
			jerr("Couldn't draw frames or text on the image");
			image = draw(false);
		}

		cv::imshow("final", image.value());
		cv::waitKey();
		cv::destroyAllWindows();
	}

	vector<string> page::get_text(ocr::dictionary& dictionary)
	{
		// Clear text
		text.clear();
		text.resize(lines.size());

		// Classify lines
		for (int line = 0; line < lines.size(); line++)
		{
			// Get blobs
			vector<ocr::blob> blobs = lines[line].get_blobs();
			Size image_size = dictionary.image_size();

			// Process blobs
			for (ocr::blob blob : blobs)
			{
				// Preprocess and classify
				Mat sign_image = image(blob.get_roi());
				resize(sign_image, image_size);
				conv3to1(sign_image);
				blur(sign_image, sign_image, cv::Size(3, 3));
				text[line].push_back(dictionary.classify(sign_image).c);
			}
		}

		return text;
	}

	vector<string> page::get_text_extended(ocr::dictionary& dictionary)
	{
		// Clear text
		text.clear();
		text.resize(lines.size());
		jinf("Recognized text:");

		// Classify lines
		for (int line = 0; line < lines.size(); line++)
		{
			// Get blobs
			vector<ocr::blob> blobs = lines[line].get_blobs();

			// Make check mat
			Size image_size = dictionary.image_size();
			Mat check_image = Mat(((uint)blobs.size() + 1) * image_size.height, (dictionary.size() + 1) * image_size.width, CV_8UC3);

			// Get line's images and draw them
			vector<Mat> images;

			for (ocr::blob blob : blobs)
			{
				Mat sign_image = image(blob.get_roi());
				resize(sign_image, image_size);
				conv3to1(sign_image);
				blur(sign_image, sign_image, cv::Size(3, 3));
				images.push_back(sign_image);
			}

			draw_column(images, check_image, Point(0, image_size.height), one_white);

			// Draw signs
			vector<ocr::sign> signs = dictionary.get_signs();
			vector<cv::Mat> sign_images;

			for (ocr::sign& sign : signs)
				sign_images.push_back(sign.get_heatmap());

			draw_row(sign_images, check_image, Point(image_size.width, 0), one_white);

			// Classify images and draw gradients
			for (int it = 0; it < images.size(); it++)
			{
				pair<ocr::result, vector<Mat>> res = dictionary.ext_classify(images[it]);
				text[line].push_back(res.first.c);
				draw_row(res.second, check_image, Point(image_size.width, image_size.height * (it + 1)), one_redgreen);
			}

			// Print results
			cout << text[line] << endl;

			// Show the image
			resize(check_image, 2.5, 2.5);
			imshow("line " + std::to_string(line), check_image);
			cv::waitKey();
		}

		cv::destroyAllWindows();

		return text;
	}

	vector<map<char, vector<Mat>>> page::get_training_data()
	{
		vector<map<char, vector<Mat>>> data;

		// Check if the text matches blobs
		if (!check_text())
		{
			journal("Text check failed", log_error);
			return data;
		}

		data.resize(lines.size());

		// Append training data
		for (size_t l = 0; l < lines.size(); l++)
		{
			vector<blob> blobs = lines[l].get_blobs();

			for (size_t b = 0; b < blobs.size(); b++)
				data[l][text[l][b]].push_back(image(blobs[b].get_roi()));
		}

		return data;
	}

	optional<Mat> page::draw(bool draw_letters)
	{
		// Check if the text matches blobs
		if (draw_letters && !check_text())
			return optional<Mat>();

		// Create image container
		Mat draw_image = conv1to3(image);

		// Draw lines
		for (size_t l = 0; l < lines.size(); l++)
		{
			// Draw the frame
			blob frame = lines[l].get_frame();
			frame.draw(draw_image, Vec3b(0, 255, 0));

			if (draw_letters)
				putText(draw_image, String("line " + to_string(l)), Point(20, frame._r.y), cv::FONT_HERSHEY_DUPLEX, 0.8, Vec3b(255, 255, 255));

			vector<blob> blobs = lines[l].get_blobs();

			// Draw blobs
			for (size_t b = 0; b < blobs.size(); b++)
			{
				blob bl = blobs[b];
				bl.draw(draw_image, Vec3b(0, 0, 255));

				if (draw_letters)
					putText(draw_image, string(1, text[l][b]), Point(bl._l.x, bl._r.y + 15), FONT_HERSHEY_SIMPLEX, 1, Vec3b(0, 255, 255));
			}
		}

		// Return the image optional
		return draw_image;
	}
}