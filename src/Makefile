CFLAG = -Wall -Werror -std=c++14 -O
all:	Task1 Task2 Task3 Task4 Task5 Adjust

.PHONY:	all

Task1:	Task1.o Task1filter.o
	g++ $(CFLAG) -o Task1 Task1.o Task1filter.o
Task2:	Task2.o Task1filter.o
	g++ $(CFLAG) -o Task2 Task2.o Task1filter.o
Task3:	Task3.o Task1filter.o
	g++ $(CFLAG) -o Task3 Task3.o Task1filter.o
Task4:	Task4.o Task1filter.o
	g++ $(CFLAG) -o Task4 Task4.o Task1filter.o
Task5:	Task5.o Task1filter.o
	g++ $(CFLAG) -o Task5 Task5.o Task1filter.o
Adjust:	Adjust.o
	g++ $(CFLAG) -o Adjust Adjust.o
Task1filter.o:	Task1filter.cpp
	g++ $(CFLAG) -c -g Task1filter.cpp
Task1.o:	Task1.cpp
	g++ $(CFLAG) -c -g Task1.cpp
Task2.o:	Task2.cpp
	g++ $(CFLAG) -c -g Task2.cpp
Task3.o:	Task3.cpp
	g++ $(CFLAG) -c -g Task3.cpp
Task4.o:	Task4.cpp
	g++ $(CFLAG) -c -g Task4.cpp
Task5.o:	Task5.cpp
	g++ $(CFLAG) -c -g Task5.cpp
Adjust.o:	Adjust.cpp
	g++ $(CFLAG) -c -g Adjust.cpp

clear:	
	rm -f *.o Task? Adjust fifo* *.txt output/*.txt clean/*.txt
