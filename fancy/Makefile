.RECIPEPREFIX = >

all: icc-pretty icc-fast day-22-2 tiny

icc-pretty: Makefile icc-pretty.cc
> g++ -std=c++17 -O3 -o icc-pretty icc-pretty.cc

icc-fast: Makefile icc-fast.cc
> g++ -std=c++17 -O3 -Wno-return-type -o icc-fast icc-fast.cc

day-22-2: Makefile day-22-2.cc
> g++ -std=c++17 -O3 -o day-22-2 day-22-2.cc
