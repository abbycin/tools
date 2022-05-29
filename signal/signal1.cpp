/*********************************************************
	  File Name:signal.cpp
	  Author: Abby Cin
	  Mail: abbytsing@gmail.com
	  Created Time: Sun 17 Jul 2016 04:05:11 PM CST
**********************************************************/

#include <iostream>
#include <functional>
#include "signal1.h"

struct T {
	void print(int x)
	{
		std::cout << "member function: " << x << "\n";
	}
	static void show(int x)
	{
		std::cout << "static member function: " << x << std::endl;
	}
};

void print(int x)
{
	std::cout << "free function: " << x << "\n";
}

int main()
{
	using nm::signal::Signal;
	T t;
	Signal<void(int)> sig;
	auto m = sig.connect<T, &T::print>(&t);
	auto handle = sig.connect([](int x)
				  { printf("anonymous function: %d\n", x); });
	sig.connect(&print);
	sig.connect(&T::show);
	sig.emit(40);
	std::cout << "----------------------\n";
	// sig.disconnect<T, &T::print>(&t);
	sig.disconnect(handle);
	sig.disconnect(&print);
	sig.disconnect(&T::show);
	sig.emit(30);
	sig.disconnect(m);
}
