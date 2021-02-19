#!/bin/bash

set -eu

# g++ main.cpp -g -std=c++11 -fsanitize=address -fsanitize=undefined -Wall -Wextra -Wshadow -DADJUST -Iinclude

g++ dump_data.cpp -o dump_data.out -O3 -std=c++11 -Iinclude

# ./a.out < ../input/example.in

echo 1
./dump_data.out ../input/first_adventure.in first_adventure_latency.tsv first_adventure_unit.tsv

echo 2
./dump_data.out ../input/second_adventure.in second_adventure_latency.tsv second_adventure_unit.tsv

echo 3
./dump_data.out ../input/third_adventure.in third_adventure_latency.tsv third_adventure_unit.tsv

echo 4
./dump_data.out ../input/fourth_adventure.in fourth_adventure_latency.tsv fourth_adventure_unit.tsv
