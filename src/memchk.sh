#!/bin/sh

#valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./Task1 data/wlist_match1.txt clean/wlist_match1-task1-clean-exe.txt
#valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./Task2 data/wlist_match1.txt output/wlist_match1-task2.txt
#valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./Task3 data/wlist_match1.txt output/wlist_match1-task3.txt
valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./Task4 data/wlist_match1.txt output/wlist_match1-task4.txt
# valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./Task5 data/wlist_match1.txt output/wlist_match1-task5.txt
