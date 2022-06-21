#!/bin/sh

set -x

rm precompiler donkey*
icpc -o precompiler ./precompiler.cpp -std=c++17
icpc -E -P sample_lasers.cpp | ./precompiler > donkey.cpp
icpc -o donkey ./donkey.cpp -std=c++17
./donkey

rm donkey*
icpc -E -P sample_simple.cpp | ./precompiler > donkey.cpp
icpc -o donkey ./donkey.cpp -std=c++17
./donkey
