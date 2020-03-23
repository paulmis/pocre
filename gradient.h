#pragma once

namespace ocr
{
	class gradient
	{
	private:

		size_t width, height;
		std::vector<std::vector<float>> heatmap;

	public:

		gradient();
		gradient(std::string path);

		bool load(std::string path);
		bool save(std::string path);

		float at(float y, float x);
	};
}

