#!/bin/sh

FILE=`echo "$1" | cut -d'/' -f2 | cut -d'.' -f1`
CLEAN_FILE=`echo "clean/$FILE"`-task1-clean.txt
echo "Task1: Start......"
cat $1 | sed 's/[^[:alnum:]]//g' | sed -nr '/^.{3,15}$/p' | sort | uniq | shuf | sort -k 1.3 > $CLEAN_FILE
echo "Task1: Done......"
wc -l $CLEAN_FILE
