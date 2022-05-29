/*********************************************************
	  File Name: test.cpp
	  Author: Abby Cin
	  Mail: abbytsing@gmail.com
	  Created Time: Sat 13 May 2017 09:15:08 AM CST
**********************************************************/

#include "variant.h"
#include <iostream>

using namespace std;
using namespace nm;

struct foo {
	foo() = default;
	foo(int x, int y) : x_(x), y_(y)
	{
	}
	~foo()
	{
		printf("--->>> %s\n", __func__);
	};
	void show()
	{
		printf("x: %d\ty: %d\n", x_, y_);
	}
	int x_;
	int y_;
};

struct bar {
	bar() = default;
	virtual ~bar()
	{
		printf("--->>> %s\n", __func__);
	}
	virtual void foo()
	{
		printf("bar\n");
	}
};

struct baz : public bar {
	~baz() override
	{
		printf("--->>> %s\n", __func__);
	}
	void foo() override
	{
		printf("baz\n");
	}
};

variant<std::string> make_variant()
{
	return variant<std::string> { std::string { "test move" } };
}

struct Operation {
	size_t operator()(size_t x)
	{
		cout << "size_t = ";
		return x;
	}
	size_t operator()(const string &x)
	{
		cout << "string.length = ";
		return x.size();
	}
	double operator()(double x)
	{
		cout << "double = ";
		return x;
	}
};

struct Operation2 : public variant_visitor<string> {
	string operator()(size_t x)
	{
		cout << "size_t = ";
		return to_string(x);
	}
	string operator()(const string &x)
	{
		cout << "string = ";
		return x;
	}
	string operator()(double x)
	{
		cout << "double = ";
		return to_string(x);
	}
};

int main()
{
	variant<bar, int, foo, std::string, double> va { bar() };
	va.get<bar>().foo(); // print 'bar'
	va.set<int>(233);
	cout << va.get<int>() << '\n';
	va.set<std::string>(std::string("+1s"));
	cout << va.get<std::string>() << '\n';
	va.set<foo>(foo());
	va.emplace<foo>(1, 2);
	va.get<foo>().show(); // test emplace
	va.set<double>(2.333);
	va.set<std::string>("233333333333333333333333");
	variant<bar, int, foo, std::string, double> v(va); // test copy ctor
	cout << v.get<std::string>() << '\n';
	variant<std::string> vv(make_variant()); // test move ctor
	cout << vv.get<std::string>() << '\n';
	v = std::move(va);
	cout << v.get<std::string>() << '\n';
	variant<bar *> b { static_cast<bar *>(new baz {}) };
	bar *ba = b.get<bar *>();
	ba->bar::foo();
	dynamic_cast<baz *>(ba)->foo();
	delete dynamic_cast<baz *>(ba);
	cout << (v != va ? "v not equal va" : " v equal va") << '\n';
	v = 2.33;
	cout << (v == va ? "v equal va" : " v not equal va") << '\n';
	variant<std::string, int> vx = "2.33";
	std::string s("-1s");
	vx = 233;
	vx = s;
	cout << s.size() << '\n';
	cout << vx.get<std::string>() << '\n';
	// test call
	variant<int, float, std::string, const char *> vc = 2.33;
	vc.call([](int x) { printf("int = %d\n", x); },
		[](float x) { printf("float = %f\n", x); },
		[](std::string x) { printf("string = %s\n", x.c_str()); },
		[](const char *x) { printf("const char* = %s\n", x); });
	vc = "maybe string";
	auto bar = [](std::string x) { printf("string = %s\n", x.c_str()); };
	vc.call(bar);
	vc.call([](const double) { printf("never print\n"); });
	// test apply

	{
		// user should ensure no conversion happen
		variant<string, double, size_t> app = 2.33;
		app.apply<size_t>(Operation {}); // print 'double = '
		cout << '\n';
	}
	{
		variant<string, double, size_t> app;
		app.emplace<double>(2.33);
		auto res = app.apply<double>(Operation {});
		cout << res << '\n';
	}
	{
		variant<string, double, size_t> app;
		app.apply<size_t>(Operation {}); // nothing
		app = 2.333;
		cout << app.apply(Operation2 {}) << '\n';
		cout << variant<int, double, string> { "test operator<<\n" };
		auto v = nm::make_overload([](size_t x) { cout << x; },
					   [](string &x) { cout << x; },
					   [](double x) { cout << x; });
		app.apply<void>(v);
		cout << '\n';
	}
}
