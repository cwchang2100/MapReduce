#!/bin/sh
rm -f fifo*
rm -f priority.txt
rm -f cpu_time.txt
#./Task4 data/wlist_match1.txt output/wlist_match1-task4.txt
./Task4 data/wlist_all.txt output/wlist_all-task4.txt
# wc -l output/wlist_all-task4.txt
