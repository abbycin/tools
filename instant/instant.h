// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Author: Abby Cin
 * Mail: abbytsing@gmail.com
 * Create Time: 2023-10-13 10:20:58
 */

#ifndef INSTANT_20231013102058
#define INSTANT_20231013102058

#include <chrono>

namespace nm
{
class Instant {
public:
	static Instant now()
	{
		return { std::chrono::steady_clock::now() };
	}

	auto elapse_usec()
	{
		return diff().count() / 1e3;
	}

	auto elapse_ms()
	{
		return diff().count() / 1e6;
	}

	auto elapse_sec()
	{
		return diff().count() / 1e9;
	}

	auto elapse_min()
	{
		return elapse_sec() / 60.0;
	}

private:
	using timepoint = std::chrono::steady_clock::time_point;
	Instant(timepoint now) : tp_ { now }
	{
	}

	auto diff() -> std::chrono::duration<double, std::nano>
	{
		return std::chrono::steady_clock::now() - tp_;
	}

	timepoint tp_;
};
}

#endif // INSTANT_20231013102058