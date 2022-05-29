/*********************************************************
	  File Name:signal_call_test.cpp
	  Author: Abby Cin
	  Mail: abbytsing@gmail.com
	  Created Time: Thu 12 May 2016 07:01:29 PM CST
**********************************************************/

#include <iostream>
#include "signal_call.h"

class Test {
public:
	void p(int x)
	{
		std::cout << x << std::endl;
	}

	void show(int x)
	{
		std::cout << "show => " << x << std::endl;
	}
};

int main()
{
	Test t;
	nm::Signal<decltype(&Test::p)> s;
	s.connect(&t, &Test::p);
	s.connect(&t, &Test::show);
	s.emit(10);

	s.disconnect(&Test::show);
	s.emit(10);

	nm::Signal<int (*)(int)> s1;
	auto handle = s1.connect(
		[](int x) -> int
		{
			std::cout << x << std::endl;
			return x;
		});
	s1.emit(11);
	s1.disconnect(handle);
	s1.emit(12);
}
