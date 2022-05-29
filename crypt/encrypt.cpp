/*********************************************************
	  File Name: encrypt.cpp
	  Author: Abby Cin
	  Mail: abbytsing@gmail.com
	  Created Time: Sat 17 Dec 2016 03:52:46 PM CST
**********************************************************/

#include <iostream>
#include <fstream>
#include "crypto.h"

void cb(int magic, std::string &s)
{
	for (auto &x : s)
		x += magic;
}

int main(int argc, char *argv[])
{
	if (argc != 4) {
		fprintf(stderr, "%s magic words path\n", argv[0]);
		return 1;
	}
	int magic = 0;
	try {
		magic = std::stoi(argv[1]);
	}
	catch (const std::logic_error &) {
		std::cout << "Invalid magic number\n";
		return 1;
	}
	std::ifstream in(argv[3]);
	if (in.is_open()) {
		std::cerr << "File exists, please choose another file.\n";
		return 1;
	}
	std::ofstream of(argv[3]);
	if (!of.is_open()) {
		std::cerr << "Can't open file: " << argv[3] << std::endl;
		return 1;
	}
	Crypto crypto;
	auto res = crypto.encode(argv[2], magic, cb);
	of.write("\3", 1);
	of.write("\0", 1);
	of.write("\t", 1);
	of << res;
	of.close();
}
