/*********************************************************
	  File Name:test.cpp
	  Author: Abby Cin
	  Mail: abbytsing@gmail.com
	  Created Time: Mon 21 Nov 2016 09:55:05 PM CST
**********************************************************/

#include "fake_variant.h"
#include <iostream>
#include <vector>
#include <string>

using namespace std;

int main()
{
	struct NonCopyMoveable {
		NonCopyMoveable(int x) : data_(x)
		{
		}
		NonCopyMoveable(const NonCopyMoveable &) = delete;
		NonCopyMoveable(NonCopyMoveable &&) = delete;
		NonCopyMoveable &operator=(const NonCopyMoveable &) = delete;
		NonCopyMoveable &operator=(NonCopyMoveable &&) = delete;
		int data_;
	};
	// ignore extra `int`
	nm::FakeVariant<int,
			int,
			NonCopyMoveable,
			int,
			const char *,
			double,
			vector<string>>
		va;
	va.set<int>(10);
	va.set<NonCopyMoveable>(309);
	va.set<const char *>("+1s");
	va.set<double>(3.09);
	// call `set(const T&)`
	va.set<vector<string>>({ "old", "value" });
	// call `set(Para&&...)`
	auto res = va.set<vector<string>, std::initializer_list<string>>(
		{ "this", " is", " a", " test." });
	if (!res)
		cout << "override exist value!\n";
	cout << va.get<int>() << endl;
	cout << va.get<NonCopyMoveable>().data_ << endl;
	cout << va.get<const char *>() << endl;
	cout << va.get<double>() << endl;
	for (const auto &x : va.get<vector<string>>())
		cout << x;
	cout << endl;
}
