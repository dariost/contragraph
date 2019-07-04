#!/bin/bash -xe

cd "${0%/*}"
rm -rf output/*
mkdir -p output
g++ -o _mincut.elf -std=c++17 -O3 -march=native -I../src/ mincut.cpp
./_mincut.elf output < mincut_input.txt
cd output
for i in *.dot; do
    dot -Tsvg $i > $(basename $i .dot).svg
    rm $i
done
