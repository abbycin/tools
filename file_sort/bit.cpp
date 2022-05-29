/*********************************************************
	  File Name:bit.cpp
	  Author: Abby Cin
	  Mail: abbytsing@gmail.com
	  Created Time: Sat 01 Oct 2016 09:58:50 AM CST
**********************************************************/

#include <iostream>
#include <chrono>
#include <bitset>

int main(int argc, char *argv[])
{
	if (argc != 3) {
		fprintf(stderr, "%s input output\n", argv[0]);
		return 1;
	}
	auto beg = std::chrono::steady_clock::now();
	const int size = 1'000'000;
	const int max_scan = size / 2 + 1;
	std::bitset<max_scan> bit_map;
	bit_map.reset();
	int duplicate[max_scan] = { 0 };

	FILE *fin = fopen(argv[1], "r");
	int n;

	while (fscanf(fin, "%d\n", &n) != EOF) {
		if (n < max_scan) {
			if (bit_map.test(n))
				duplicate[n] += 1;
			else
				bit_map.set(n, true);
		}
	}

	FILE *fout = fopen(argv[2], "w");
	int i;

	for (i = 0; i < max_scan; ++i) {
		if (bit_map[i] == true) {
			for (int j = 0; j < duplicate[i]; ++j)
				fprintf(fout, "%d\n", i);
			fprintf(fout, "%d\n", i);
		}
	}

	fseek(fin, 0, SEEK_SET);
	bit_map.reset();
	for (i = 0; i < max_scan; ++i)
		duplicate[i] = 0;

	while (fscanf(fin, "%d\n", &n) != EOF) {
		if (n >= max_scan && n <= size) {
			n -= max_scan;
			if (bit_map.test(n))
				duplicate[n] += 1;
			else
				bit_map.set(n, true);
		}
	}

	for (i = 0; i < max_scan; ++i) {
		if (bit_map[i] == true) {
			for (int j = 0; j < duplicate[i]; ++j)
				fprintf(fout, "%d\n", i + max_scan);
			fprintf(fout, "%d\n", i + max_scan);
		}
	}
	fclose(fin);
	fclose(fout);
	auto end = std::chrono::steady_clock::now();
	auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(end -
									 beg);
	std::cout << dur.count() << "ms\n";
}
