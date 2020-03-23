#include "pch.h"
#include "line.h"

namespace ocr
{
	line::line()
	{}

	void line::insert(blob _new)
	{
		if (blobs.size() == 0)
			frame = _new;

		blobs.insert(_new);
		frame.insert(_new);
	}

	vector<blob> line::get_blobs()
	{
		vector<blob> ret;

		for (blob b : blobs)
			ret.push_back(b);

		return ret;
	}

	blob line::get_frame()
	{
		return frame;
	}

	size_t line::height()
	{
		return (frame._l.y + frame._r.y) / 2;
	}

	size_t line::size()
	{
		return blobs.size();
	}

	bool line::inside_vertically(int y)
	{
		return (y > frame._l.y&& y < frame._r.y);
	}
}