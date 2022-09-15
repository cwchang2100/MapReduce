#!/bin/sh
#./Task1.sh data/wlist_match1.txt
# wc -l clean/wlist_match1-task1-clean.txt
#./Task1 data/wlist_match1.txt clean/wlist_match1-task1-clean-exe.txt
./Task1 data/wlist_all.txt clean/wlist_all-clean.txt
wc -l clean/wlist_all-clean.txt
