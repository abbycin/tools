/*********************************************************
	  File Name:test_hash.cpp
	  Author: Abby Cin
	  Mail: abbytsing@gmail.com
	  Created Time: Mon 01 Aug 2016 08:53:17 PM CST
**********************************************************/

#include <iostream>
#include "hash.h"

int main()
{
	nm::hash::HashTable h(true);
	auto hash = h.insert("key", "value1");
	h.insert("key", "value2");
	h.insert("key", "value3");
	h.insert("key", "value4");
	auto iter = h.search("key");
	auto travel = [](nm::hash::HashTable::iterator &iter)
	{
		while (iter.is_valid()) {
			std::cout << iter->value << std::endl;
			++iter;
		}
	};
	travel(iter); // iterator no longer valid
	iter = h.search("key");
	if (iter.is_valid())
		std::cout << "sibling: " << iter->dup_count << std::endl;
	h.remove("key", hash);
	h.remove("key", hash);
	iter = h.search("key");
	if (iter.is_valid())
		std::cout << "sibling: " << iter->dup_count << std::endl;
	h.insert("key", "value5");
	h.insert("key", "value6");
	travel(iter);
	iter = h.search("key");
	if (iter.is_valid())
		std::cout << "sibling: " << iter->dup_count << std::endl;
	h.remove_all("key", hash); // remove sibling and node itself
}
