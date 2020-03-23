#pragma once

namespace ocr
{
	struct result
	{
		char c;
		double val;

		result();
		result(char _c);
		result(char _c, double _val);

		bool operator<(const result& other) const;
		bool operator>(const result& other) const;
		result& operator=(const result& other);
		result& operator+=(const double& _val);
	};
}

#define DEFAULT_RESULT ocr::result('?', 0)

std::ostream& operator<<(std::ostream& os, const ocr::result& res);
