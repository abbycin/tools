/*********************************************************
	  File Name:bs.cpp
	  Author: Abby Cin
	  Mail: abbytsing@gmail.com
	  Created Time: Tue 27 Sep 2016 04:30:51 PM CST
**********************************************************/

#include <iostream>
#include <cstring>
#include <algorithm>
#include <chrono>

#define failure(func, line)                                                    \
	do {                                                                   \
		fprintf(stderr, "%s : %d\t", func, line);                      \
		perror(func);                                                  \
		std::terminate();                                              \
	}                                                                      \
	while (0)

class Bigdata {
public:
	Bigdata(long limit, const std::string &input, const std::string &output)
		: slot_(limit), in_(input), out_(output), count_(0)
	{
	}

	void sort()
	{
		split_files();
		merge_files();
		do_clean();
	}

private:
	const long slot_;
	std::string in_, out_;
	long count_;

	FILE *get_tmpfd()
	{
		char name[50];
		sprintf(name, "tmp_%ld", count_++);
		FILE *tmpfd = fopen(name, "w");
		if (tmpfd == nullptr)
			failure(__func__, __LINE__);
		return tmpfd;
	}

	void split_files()
	{
		FILE *fd = fopen(in_.c_str(), "r");
		if (fd == nullptr)
			failure(__func__, __LINE__);
		long n = 0;
		FILE *tmpfd = get_tmpfd();
		long local_count = 0;
		long *numbers_ = new long[slot_];
		while (fscanf(fd, "%ld\n", &n) != EOF) {
			numbers_[local_count++] = n;
			if (local_count == slot_) {
				local_count = 0;
				std::sort(numbers_, numbers_ + slot_);
				for (long i = 0; i < slot_; ++i)
					fprintf(tmpfd, "%ld\n", numbers_[i]);
				memset(numbers_, 0, sizeof(long) * slot_);
				fclose(tmpfd);
				tmpfd = get_tmpfd();
			}
		}
		if (local_count != 0) {
			std::sort(numbers_, numbers_ + slot_);
			for (long i = 0; i < slot_; ++i)
				fprintf(tmpfd, "%ld\n", numbers_[i]);
		}
		delete[] numbers_;
		fclose(tmpfd);
		fclose(fd);
	}

	void merge_files()
	{
		FILE *fd = fopen(out_.c_str(), "w");
		if (fd == nullptr)
			failure(__func__, __LINE__);
		std::vector<FILE *> fds(count_);
		std::vector<long> data(count_);
		std::vector<bool> done(count_);
		char name[50];
		for (long i = 0; i < count_; ++i) {
			done[i] = false;
			data[i] = 0;
			sprintf(name, "tmp_%ld", i);
			fds[i] = fopen(name, "r");
			if (fscanf(fds[i], "%ld\n", &data[i]) == EOF) {
				fclose(fds[i]);
				remove(name);
				done[i] = true;
			}
		}
		while (true) {
			long j = 0;
			while (j < count_ && done[j])
				++j;
			if (j >= count_)
				break;
			long minimum = data[j];
			for (long i = j + 1; i < count_; ++i) {
				if (!done[i] && minimum > data[i]) {
					minimum = data[i];
					j = i;
				}
			}
			fprintf(fd, "%ld\n", minimum);
			if (fscanf(fds[j], "%ld\n", &data[j]) == EOF) {
				fclose(fds[j]);
				sprintf(name, "tmp_%ld", j);
				remove(name);
				done[j] = true;
			}
		}
		fclose(fd);
	}

	void do_clean()
	{
		char name[50];
		for (long i = 0; i < count_; ++i) {
			sprintf(name, "tmp_%ld", i);
			remove(name);
		}
	}
};

int main(int argc, char *argv[])
{
	if (argc != 4) {
		fprintf(stderr, "%s num_per_file input output\n", argv[0]);
		return 1;
	}
	auto beg = std::chrono::steady_clock::now();
	Bigdata bigdata(std::stol(argv[1]), argv[2], argv[3]);
	bigdata.sort();
	auto end = std::chrono::steady_clock::now();
	auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(end -
									 beg);
	std::cout << dur.count() << "ms\n";
}
