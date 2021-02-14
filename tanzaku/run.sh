#!/bin/bash

set -eu

# g++ main.cpp -g -std=c++11 -fsanitize=address -fsanitize=undefined -Wall -Wextra -Wshadow -DADJUST
g++ main.cpp -O3 -std=c++11

# ./a.out < ../input/example.in

echo 1
./a.out < ../input/first_adventure.in > first_adventure.out

echo 2
./a.out < ../input/second_adventure.in > second_adventure.out

echo 3
./a.out < ../input/third_adventure.in > third_adventure.out

echo 4
./a.out < ../input/fourth_adventure.in > fourth_adventure.out

