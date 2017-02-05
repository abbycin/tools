#!/bin/bash

compile()
{
    for i in $(ls *.cpp)
    do
        out=$(echo $i | sed 's/.cpp//')
        g++ -std=c++14 $i -o $out -O3 -s
    done
}

mkdata()
{
    ./rand 1000000 data
}

clean()
{
    echo -e "\e[32mcleaning...\e[0m"
    rm -f rand bs bit data bs_out bit_out
    echo -e "\e[32mexit.\e[0m"
}

run_bi()
{
    # bitset's template argument must be deternined at compile time
    echo "bit test is running..."
    ./bit data bit_out
    echo "done!"
}

run_bs()
{
    echo "bs test is running..."
    ./bs 500000 data bs_out
    echo "done!"
}

big_file_test()
{
    echo "bs test is running on big file..."
    rm -f data bs_out
    ./rand 100000000 data
    ./bs 2000000 data bs_out
    echo "done!"
}

main()
{
    compile
    mkdata
    run_bi
    run_bs
    echo "--------------------"
    du -h *
    echo "--------------------"
    echo "now test big file sort..."
    big_file_test
    echo "--------------------"
    du -h *
    echo "--------------------"
    clean
}

main
