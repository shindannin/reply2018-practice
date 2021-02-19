#!/bin/bash

set -eu

# g++ main.cpp -g -std=c++11 -fsanitize=address -fsanitize=undefined -Wall -Wextra -Wshadow -DADJUST -Iinclude
g++ main.cpp -O3 -std=c++11 -Iinclude

g++ validate.cpp -o validate.out -O3 -std=c++11 -Iinclude
g++ gen_answer.cpp -o gen_answer.out -O3 -std=c++11 -Iinclude


# ./a.out < ../input/example.in

# echo 1
# ./a.out < ../input/first_adventure.in > first_adventure_latency.tsv
# ./a.out < ../input/first_adventure.in > first_adventure.out

# echo 2
# # ./a.out < ../input/second_adventure.in > second_adventure_latency.tsv
# ./a.out < ../input/second_adventure.in > second_adventure.out

echo 3
# ./a.out < ../input/third_adventure.in > third_adventure_latency.tsv
./a.out < ../input/third_adventure.in > third_adventure.out
./gen_answer.out ../input/third_adventure.in > third_adventure.out
./validate.out ../input/third_adventure.in third_adventure.out

# echo 4
# # # ./a.out < ../input/fourth_adventure.in > fourth_adventure_latency.tsv
# # ./a.out < ../input/fourth_adventure.in > fourth_adventure_service.tsv
# ./a.out < ../input/fourth_adventure.in
# ./gen_answer.out ../input/fourth_adventure.in > fourth_adventure.out
# ./validate.out ../input/fourth_adventure.in fourth_adventure.out

