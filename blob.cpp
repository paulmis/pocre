#include "pch.h"
#include "blob.h"

namespace ocr
{
	blob::blob()
		: _l(INT_MAX, INT_MAX), _r(INT_MIN, INT_MIN) {}

	blob::blob(Point l, Point r)
		: _l(l), _r(r) {}

	void blob::draw(Mat im, Vec3b color)
	{
		Point p = _l;

		while (p.y++ < _r.y - 1) im.at<Vec3b>(p) = color;
		while (p.x++ < _r.x - 1) im.at<Vec3b>(p) = color;
		while (p.y-- > _l.y + 1) im.at<Vec3b>(p) = color;
		while (p.x-- > _l.x + 1) im.at<Vec3b>(p) = color;
	}

	void blob::insert(Point point)
	{
		_l = Point(min(_l.x, point.x), min(_l.y, point.y));
		_r = Point(max(_r.x, point.x), max(_r.y, point.y));
	}

	void blob::insert(blob b)
	{
		insert(b._l), insert(b._r);
	}

	void blob::buffer(Size image_size, int buffer_size)
	{
		_l.x = max(0, _l.x - buffer_size);
		_l.y = max(0, _l.y - buffer_size);
		_r.x = min(image_size.width - 1, _r.x + buffer_size);
		_r.y = min(image_size.height - 1, _r.y + buffer_size);
	}

	// Creates a blob that encapsulates a sign and returns all its points
	vector<Point> blob::make(Point start, Mat image, vector<vector<bool>>& visited, function<bool(float)> eligible, Rect& frame)
	{
		// Initialize BFS variables
		static vector<Point> adj = { {-1, 0}, {0, -1}, {1, 0}, {0, 1}, {-1, -1}, {1, -1}, {-1, 1}, {1, 1} };
		queue<Point> queue;
		vector<Point> points;
		queue.push(start);
		visited[start.y][start.x] = true;

		// Run the BFS
		while (!queue.empty())
		{
			// Get and pop the front of the queue, insert it into the blob and increment points
			Point cur = queue.front(); queue.pop();
			insert(cur);
			points.push_back(cur);

			// Iterate through adjecent points
			for (Point mov : adj)
			{
				Point nxt = Point(cur.x + mov.x, cur.y + mov.y);
				if (frame.contains(nxt) && !visited[nxt.y][nxt.x] && eligible(image.at<float>(nxt)))
				{
					visited[nxt.y][nxt.x] = true;
					queue.push(nxt);
				}
			}
		}

		return points;
	}

	Rect blob::get_roi()
	{
		return Rect(_l.x, _l.y, width(), height());
	}

	uint blob::width()
	{
		return _r.x - _l.x + 1;
	}

	uint blob::height()
	{
		return _r.y - _l.y + 1;
	}

	int blob::x_mid()
	{
		return (_l.x + _r.x) / 2;
	}

	int blob::y_mid()
	{
		return (_l.y + _r.y) / 2;
	}

	Point blob::mid()
	{
		return { x_mid(), y_mid() };
	}

	uint blob::size()
	{
		return height() * width();
	}

	double blob::get_hw()
	{
		return (double)height() / (double)width();
	}

	bool blob::inside(Point pt)
	{
		return pt.x >= _l.x && pt.x <= _r.x && pt.y >= _l.y && pt.y <= _r.y;
	}
}