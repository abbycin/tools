// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Author: Abby Cin
 * Mail: abbytsing@gmail.com
 * Create Time: 2023-10-13 10:21:10
 */

#include "instant.h"
#include <thread>

void die_on_false(bool ok)
{
	if (!ok)
		abort();
}

int main()
{
	using namespace std::chrono_literals;
	{
		auto e = nm::Instant::now();
		std::this_thread::sleep_for(10us);
		die_on_false(e.elapse_usec() > 10);
	}
	{
		auto e = nm::Instant::now();
		std::this_thread::sleep_for(10ms);
		die_on_false(e.elapse_ms() >= 10);
	}
	{
		auto e = nm::Instant::now();
		std::this_thread::sleep_for(2s);
		die_on_false(e.elapse_sec() >= 2);
	}
	{
		auto e = nm::Instant::now();
		std::this_thread::sleep_for(61s);
		die_on_false(e.elapse_min() > 1);
	}
}