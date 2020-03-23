#pragma once

namespace ocr
{
	using namespace std;

	class line
	{
	private:

		blob frame;
		set<blob> blobs;

	public:

		line();

		void insert(blob _new);

		vector<blob> get_blobs();
		blob get_frame();
		size_t height();
		size_t size();
		bool inside_vertically(int y);
	};
}