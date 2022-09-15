#!/bin/sh
rm -f fifo*
rm -f priority.txt
rm -f cpu_time.txt
./Adjust ./Task4 data/wlist_all.txt output/wlist_all-task4.txt > task4_cpu.log 2> task4_pri.log
# wc -l output/wlist_all-task4.txt
