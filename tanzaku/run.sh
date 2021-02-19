#!/bin/bash

set -eu

# g++ main.cpp -g -std=c++11 -fsanitize=address -fsanitize=undefined -Wall -Wextra -Wshadow -DADJUST -Iinclude

g++ validate.cpp -o validate.out -O3 -std=c++11 -Iinclude
g++ gen_answer.cpp -o gen_answer.out -O3 -std=c++11 -Iinclude

g++ solver_B.cpp -o solver_B.out -O3 -std=c++11 -Iinclude
g++ solver_CD.cpp -o solver_CD.out -O3 -std=c++11 -Iinclude

# ./a.out < ../input/example.in

# echo 1
# ./a.out < ../input/first_adventure.in > first_adventure_latency.tsv
# ./a.out < ../input/first_adventure.in > first_adventure.out

echo 2
./solver_B.out ../input/second_adventure.in second_adventure.cereal
./gen_answer.out ../input/second_adventure.in second_adventure.cereal second_adventure.out
./validate.out ../input/second_adventure.in second_adventure.out


echo 3
./solver_CD.out ../input/third_adventure.in third_adventure.cereal
./gen_answer.out ../input/third_adventure.in third_adventure.cereal third_adventure.out
./validate.out ../input/third_adventure.in third_adventure.out

echo 4
./solver_CD.out ../input/fourth_adventure.in fourth_adventure.cereal
./gen_answer.out ../input/fourth_adventure.in fourth_adventure.cereal fourth_adventure.out
./validate.out ../input/fourth_adventure.in fourth_adventure.out
