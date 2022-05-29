/*********************************************************
	  File Name: main.cpp
	  Author: Abby Cin
	  Mail: abbytsing@gmail.com
	  Created Time: Sun 03 Jun 2018 10:24:50 AM CST
**********************************************************/

#include <string>
#include <iostream>
#include "coroutine.h"

void foo(nm::Coroutine<char> *co, const std::string &str)
{
	for (auto &c : str) {
		printf("%c", c);
		co->yield();
	}
	printf("\n");
}

int main()
{
	{
		nm::Coroutine<char> co;
		co.start(foo, &co, "hello world");

		for (auto iter = co.begin(); iter != co.end(); ++iter) {
		}
	}

	{
		nm::Coroutine<int> co;
		co.start(
			[&co](int x)
			{
				int first = 1;
				co.yield(first);

				int second = 1;
				co.yield(second);

				for (int i = 0; i < x; ++i) {
					int third = first + second;
					first = second;
					second = third;
					co.yield(third);
				}
			},
			10);

		for (auto &iter : co) {
			printf("%d ", iter);
		}
		printf("\n");
	}
}
