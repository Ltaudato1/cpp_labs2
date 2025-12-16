#!/bin/sh

g++ --std=c++20 -o main main.cpp -ltbb
./main
rm main