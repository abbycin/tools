# Test (sort numbers in a big file)

### bit.cpp

this file use `std::bitset` to handle this problem, and it's efficient is small scale numbers(e.g, no more than 1M numbers).

### bs.cpp

this file works like `divide-conquer`, it split a big file into many small files, and then sort each file individually, then travel through all these small files read one number each file and find the smallset one(or biggest one) and then write it to a new file(all numbers in this file will be sorted).

### rand.cpp

this file will generate random numbers and write them to a file(some numbers in this file are duplicate).
