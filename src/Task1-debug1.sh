#!/bin/sh

FILE=`echo "$1" | cut -d'/' -f2 | cut -d'.' -f1`
STEP1=`echo "debug/$FILE"`-debug1-step1-sed.txt
STEP2=`echo "debug/$FILE"`-debug1-step2-sed-length.txt
STEP3=`echo "debug/$FILE"`-debug1-step3-sort.txt
STEP4=`echo "debug/$FILE"`-debug1-step4-uniq-clean.txt
echo "Debug1: Sed......"
cat $1 | sed 's/[^[:alnum:]]//g' > $STEP1
echo "Debug1: Sed(length)......"
cat $STEP1 | sed -nr '/^.{3,15}$/p' > $STEP2
echo "Debug1: Sort......"
cat $STEP2 | sort > $STEP3
echo "Debug1: Uniq......"
cat $STEP3 | uniq > $STEP4
echo "Debgu1: Done......"
