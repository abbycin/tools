/*********************************************************
	  File Name: test.cpp
	  Author: Abby Cin
	  Mail: abbytsing@gmail.com
	  Created Time: Tue 04 Apr 2017 06:20:35 PM CST
**********************************************************/

#include <iostream>
#include <fstream>
#include "string_ext.h"

using namespace std;
using namespace nm;

bool check(const string_ext &ip)
{
	vector<string_ext> v;
	ip.split(v, ".");
	if (v.size() != 4)
		return false;
	int count = 0;
	for (auto &x : v) {
		if (!x.is_digit())
			return false;
		if (x.size() > 3 || x.size() < 1)
			return false;
		count = 0;
		for (auto &c : x) {
			if (c > '9' || c < '0')
				return false;
			count *= 10;
			count += (c - '0');
		}
		if (count > 255)
			return false;
		// strict
		switch (x.size()) {
		case 2:
			if (count < 10)
				return false;
			break;
		case 3:
			if (count < 100)
				return false;
			break;
		default:
			break;
		}
	}
	return true;
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "%s file\n", argv[0]);
		return 0;
	}
	ifstream in(argv[1], in.in);
	if (!in.is_open()) {
		cout << "can't open file: " << argv[1] << endl;
	}
	string_ext line;
	vector<string_ext> ips;
	while (getline(in, line)) {
		line.split(ips);
	}
	in.close();
	for (auto &ip : ips) {
		ip.strip();
		if (ip.size() != 0)
			cout << (check(ip) ? "valid ip" : "invalid ip") << "\t"
			     << ip << endl;
	}
	string_ext b {
		R"del(http://gslb.miaopai.com/stream/aQVm-hdSl~OLe6Lh7Q6C9A__.mp4?yx=&refer=weibo_app&Expires=1490264147&ssig=07XsfjKJkG&KID=unistore,video)del"
	};
	string_ext pattern {
		R"del(http://(.*\.miaopai.com)/(.*)/([^?]*)(.*))del"
	};
	bool res = b.match(pattern);
	if (res)
		cout << "match\n";
	else
		cout << "not match\n";
	string host, path, param;
	if (b.extract(pattern, &host, &path, &param))
		cout << "http://" << host << "/" << path << "/" << param
		     << endl;
	else
		cout << "can't extract!\n";
	regex re { pattern };
	if (b.match(re))
		cout << "match\n";
	string_ext alnum { "ip: 233.233.wtf.233.233" };
	regex re2 {
		R"del(([a-z]{1,}).+?(\d{1,3})\.(\d{1,3})\.(.+?)\.(\d{1,3})\.(\d{1,3}))del"
	};
	int _1st, _2nd, _3rd, _4th;
	string str;

	// skip `wtf`
	// no zuo no die.
	// if(alnum.extract(re2, &str, &_1st, &_2nd, (string*)0, &_3rd, &_4th))
	// you should try these
	if (alnum.extract(re2, &str, &_1st, &_2nd, nullptr, &_3rd, &_4th))
		cout << str << ": " << _1st << "." << _2nd << "." << _3rd << "."
		     << _4th << endl;
	else
		cout << "can't extract!\n";
}
