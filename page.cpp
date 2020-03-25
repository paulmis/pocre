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
			if (lines[l].size() != text[l].size())
			{
				jerr("Missmatch in line " + std::to_string(l + 1) + ": expected " + std::to_string(lines[l].size()) + " signs but the text is " + std::to_string(text[l].size()) + " characters long");
				return false;
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
			line.erase(std::remove(line.begin(), line.end(), ':'), line.end());
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

		Rect frame(Point(), _image.size());
		vector<vector<bool>> visited(_image.rows, vector<bool>(_image.cols, false));
		for (int y = 0; y < _image.rows; y++)
			for (int x = 0; x < _image.cols; x++)
				if (!visited[y][x])
				{
					queue<Point> queue;
					vector<Point> points;
					queue.push({ x, y });
					visited[y][x] = true;

					while (!queue.empty())
					{
						Point cur = queue.front(); queue.pop();
						points.push_back(cur);

						for (Point mv : mov)
						{
							Point nxt(cur.x + mv.x, cur.y + mv.y);
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
	void page::classify(function<bool(float)> _eligible, int _min_points)
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
				if (!visited[y][x] && _eligible(image.at<float>(y, x)))
				{
					ocr::blob blob;
					vector<cv::Point> points = blob.make(cv::Point(x, y), image, visited, _eligible, frame);

					// Insert to blob if it's big enough
					if (points.size() > _min_points&& points.size() < _min_points * 15)
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

	void page::ext_classify(function<bool(float)> _eligible, size_t _min_blob, bool _draw_letters)
	{
		classify(_eligible, _min_blob);
		cv::imshow("final", draw(_draw_letters));
		cv::waitKey();
		cv::destroyAllWindows();
	}

	vector<string> page::get_text(ocr::dictionary& _dictionary)
	{
		// Clear text
		text.clear();
		text.resize(lines.size());

		// Classify subsequent blobs
		for (int l = 0; l < lines.size(); l++)
			for (ocr::blob b : lines[l].get_blobs())
			{
				// Preprocess
				Mat sign_image = image(b.get_roi());
				resize(sign_image, _dictionary.image_size());
				conv3to1(sign_image);
				blur(sign_image, sign_image, Size(3, 3));

				text[l].push_back(_dictionary.classify(sign_image).c);
			}

		return text;
	}

	vector<string> page::get_text_extended(ocr::dictionary& _dictionary)
	{
		// Clear text
		text.clear();
		text.resize(lines.size());
		jinf("Recognized text:");

		// Classify lines
		for (int l = 0; l < lines.size(); l++)
		{
			// Create a classification check image
			Size image_size = _dictionary.image_size();
			vector<blob> blobs = lines[l].get_blobs();
			Mat check_image = Mat(((uint)blobs.size() + 1) * image_size.height, (_dictionary.size() + 1) * image_size.width, CV_8UC3);

			// Get line's images and draw them
			vector<Mat> images;

			for (blob b : lines[l].get_blobs())
			{
				Mat sign_image = image(b.get_roi());
				resize(sign_image, image_size);
				conv3to1(sign_image);
				blur(sign_image, sign_image, Size(3, 3));
				images.push_back(sign_image);
			}

			draw_column(images, check_image, Point(0, image_size.height), one_white);

			// Draw signs
			vector<Mat> sign_images;
			for (sign& s : _dictionary.get_signs())
				sign_images.push_back(s.get_heatmap());
			draw_row(sign_images, check_image, Point(image_size.width, 0), one_white);

			// Classify images and draw gradients
			for (int it = 0; it < images.size(); it++)
			{
				pair<result, vector<Mat>> res = _dictionary.ext_classify(images[it]);
				text[l].push_back(res.first.c);
				draw_row(res.second, check_image, Point(image_size.width, image_size.height * (it + 1)), one_redgreen);
			}

			// Print results
			cout << "line " << l << ": " << text[l] << endl;

			// Show the image
			resize(check_image, 2.5, 2.5);
			imshow("line " + ts(l), check_image);
			waitKey();
		}

		destroyAllWindows();
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

		// Append training data
		data.resize(lines.size());
		for (size_t l = 0; l < lines.size(); l++)
		{
			vector<blob> blobs = lines[l].get_blobs();
			for (size_t b = 0; b < blobs.size(); b++)
				data[l][text[l][b]].push_back(image(blobs[b].get_roi()));
		}

		return data;
	}

	Mat page::draw(bool _draw_letters)
	{
		Mat _image = conv1to3(image);
		for (size_t l = 0; l < lines.size(); l++)
		{
			// Draw the frame
			blob frame = lines[l].get_frame();
			frame.draw(_image, green);

			// Draw blobs
			vector<blob> blobs = lines[l].get_blobs();
			for (size_t b = 0; b < blobs.size(); b++)
			{
				blob bl = blobs[b];
				bl.draw(_image, red);

				if (_draw_letters)
					putText(_image, string(1, text[l][b]), Point(bl._l.x, bl._r.y + 15), FONT_HERSHEY_SIMPLEX, 1, Vec3b(0, 255, 255));
			}

			// Draw letters
			if (_draw_letters && text.size() < l && lines[l].size() == text[l].size())
				putText(_image, String("[" + ts(l) + "]"), Point(20, frame._r.y), cv::FONT_HERSHEY_DUPLEX, 1.0, white);
		}

		return _image;
	}
}