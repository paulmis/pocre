#pragma once

namespace ocr
{
	using namespace std;
	using namespace cv;

	struct blob
	{
		blob();
		blob(Point l, Point r);

		bool operator<(const blob& other) const
		{
			return _l.x < other._l.x;
		}

		Point _l, _r;

		void draw(Mat im, Vec3b color);
		void insert(Point point);
		void insert(blob b);
		void buffer(Size image_size, int buffer_size = 1);
		vector<Point> make(Point start, Mat image, vector<vector<bool>>& visited, function<bool(float)> eligible, Rect& frame);

		Rect get_roi();
		uint width();
		uint height();
		int x_mid();
		int y_mid();
		Point mid();
		uint size();
		double get_hw();
		bool inside(Point pt);
	};
}