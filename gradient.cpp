#include "pch.h"
#include "gradient.h"

namespace ocr
{
	gradient::gradient()
	{}

	gradient::gradient(std::string path)
	{
		if (!load(path))
			journal("Gradient construction failed", log_error);
	}

	bool gradient::load(std::string path)
	{
		ifstream in(path.c_str());

		if (!in.is_open())
		{
			journal("Unable to load gradient from " + path, log_error);
			return false;
		}

		string buf;
		size_t it = 0;
		file_read(in, buf);

		height = splstr<size_t>(buf, it);
		width = splstr<size_t>(buf, it);

		heatmap.resize(height, vector<float>(width));

		for (vector<float>& row : heatmap)
		{
			file_read(in, buf);
			it = 0;

			for (float& cell : row)
				cell = splstr<float>(buf, it, '\t');
		}

		in.close();
		return true;
	}

	bool gradient::save(std::string path)
	{
		ofstream out(path.c_str());

		if (!out.is_open())
		{
			journal("Unable to save gradient to " + path, log_error);
			return false;
		}

		out << height << " " << width << "\n";

		for (vector<float>& row : heatmap)
		{
			for (float& cell : row)
				out << cell << " ";

			out << "\n";
		}

		out.close();
		return true;
	}

	float gradient::at(float y, float x)
	{
		// 1.0 causes overflow as it would be in the next interval
		int iy = (int)(y * height) - (y == 1.0);
		int ix = (int)(x * width) - (x == 1.0);

		return heatmap[iy][ix];
	}
}