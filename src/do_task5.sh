#!/bin/sh
rm -f fifo*
./Task5 data/wlist_all.txt output/wlist_all-task5.txt
wc -l output/wlist_all-task5.txt
