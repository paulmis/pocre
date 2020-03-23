#include "pch.h"
#include "result.h"

namespace ocr
{
	result::result()
	{}

	result::result(char _c)
		: c(_c), val(0) {}

	result::result(char _c, double _val)
		: c(_c), val(_val) {}

	bool result::operator<(const result& other) const
	{
		return val < other.val;
	}

	bool result::operator>(const result& other) const
	{
		return val > other.val;
	}

	result& result::operator=(const result& other)
	{
		c = other.c;
		val = other.val;
		return *this;
	}
	result& result::operator+=(const double& _val)
	{
		val += _val;
		return *this;
	}
}

std::ostream& operator<<(std::ostream& os, const ocr::result& res)
{
	os << std::fixed << std::setprecision(3);
	os << "[" << res.c << ", " << res.val << "] ";
	return os;
}
