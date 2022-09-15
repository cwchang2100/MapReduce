#!/bin/sh

FILE=`echo "$1" | cut -d'/' -f2 | cut -d'.' -f1`
STEP1=`echo "debug/$FILE"`-debug2-step1-sed.txt
STEP2=`echo "debug/$FILE"`-debug2-step2-sed-length.txt
STEP3=`echo "debug/$FILE"`-debug2-step3-sort.txt
STEP4=`echo "debug/$FILE"`-debug2-step4-uniq.txt
STEP5=`echo "debug/$FILE"`-debug2-step5-shuf.txt
STEP6=`echo "debug/$FILE"`-debug2-step6-sort-clean.txt
echo "Debug2: Sed......"
cat $1 | sed 's/[^[:alnum:]]//g' > $STEP1
echo "Debug2: Sed(Length)......"
cat $STEP1 | sed -nr '/^.{3,15}$/p' > $STEP2
echo "Debug2: Sort......"
cat $STEP2 | sort > $STEP3
echo "Debug2: Uniq......"
cat $STEP3 | uniq > $STEP4
echo "Debug2: Shuffle......"
cat $STEP4 | shuf > $STEP5
echo "Debug2: Sort(from 3rd)......"
cat $STEP5 | sort -k 1.3 > $STEP6
echo "Debgu2: Done......"
