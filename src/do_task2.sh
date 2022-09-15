#!/bin/sh
rm -f mmap* sort*
./Task2 data/wlist_all.txt output/wlist_all-task2.txt
wc -l output/wlist_all-task2.txt
