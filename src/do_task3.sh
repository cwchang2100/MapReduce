#!/bin/sh
rm -f fifo*
./Task3 data/wlist_all.txt output/wlist_all-task3.txt
wc -l output/wlist_all-task3.txt
