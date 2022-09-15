#include <iostream>
#include <string>
#include <fstream>
#include <pthread.h>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <random>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/time.h>
#include <string.h>

using namespace std;

extern vector<string> TaskFilter(string fn1);

void map3();
void reduce3(char *fn);

vector<string> Globals;
vector<vector<int>> Indexs(13);

pthread_t pids[13];

struct timeval tv;

long long start_time;
long long end_time;

long long clean_time;
long long map_time;
long long reduce_time;

int cmp(const void *x, const void *y) {
    return strcmp(Globals[*(int*)x].substr(2).c_str(), Globals[*(int*)y].substr(2).c_str());
}

void* map3Task(void *argc) {
    clockid_t cid;
    struct timespec ts;
    long long cpu_start_time;
    long long cpu_end_time;
    long long cpu_spent_time;

    int s = pthread_getcpuclockid(pthread_self(), &cid);
    if (s != 0) {
        std::cout << "clockid error!" << std::endl;
    }
    clock_gettime(cid, &ts);
    cpu_start_time = (ts.tv_sec)*1000 + (ts.tv_nsec/1000000);

    int taskid = *(int*)(argc);
    // qsort 
    qsort(&Indexs[taskid][0], Indexs[taskid].size(), sizeof(int), cmp);
	
    // write fifo
    string fifo = "fifo_" + to_string(taskid);
    ofstream of;
    of.open(fifo);

    for(size_t i = 0; i < Indexs[taskid].size(); i++) {
	of << Globals[Indexs[taskid][i]] << std::endl;
    }

    of.close();
	
    delete (int*)argc;

    clock_gettime(cid, &ts);
    cpu_end_time = (ts.tv_sec)*1000 + (ts.tv_nsec/1000000);
    cpu_spent_time = cpu_end_time - cpu_start_time;
    std::cout << "pthread (" << to_string(taskid+3) << ") CPU time: " << to_string(cpu_spent_time) << std::endl;

    return (void*)NULL;
}

void map3(){
    std::cout << "map3" << std::endl;
    // update 13 Indexs
    for(size_t i = 0; i < Globals.size(); i++) {
	Indexs[Globals[i].size()-3].push_back(i);
    }
	
    // create 13 map threads
    for(size_t i = 0; i < 13; i++) {

        // create fifo
        string fifo = "fifo_" + to_string(i);
        mkfifo(fifo.c_str(), 0777);

	int *taskid = new int(i);
	pthread_create(&pids[i], NULL, map3Task, (void*)taskid);
    }
}

void reduce3(char *fn) {
    string result = string(fn);
	
    ifstream ins[13];
    bool     eofs[13];
    string   words[13];
    ofstream out;
    string   line;
	
    std::cout << "reduce3" << std::endl;
    out.open(result);
	
    for(size_t i = 3; i <= 15; i++) {
		
 	ifstream in;
		
	string fifo = "fifo_" + to_string(i-3);
	in.open(fifo.c_str());
		
	// read first line
	if(getline(in, line)) {
            words[i-3] = line;
	    eofs[i-3]  = false;
	} else {
	    words[i-3] = "";
	    eofs[i-3]  = true;
	}
	ins[i-3] = std::move(in);
    }

    int minIdx = -1;
	
    do {
        string minStr = "";
	minIdx = -1;
		
        for(size_t i = 0; i < 13; i++) {
	    if (eofs[i] ==  false) {
	        if (minIdx == -1 || words[i].substr(2) < minStr) {
	            minIdx = i;
		    minStr = words[i].substr(2);
	        }
	    }
        }
		
        if (minIdx != -1) {
	
            out << words[minIdx] << std::endl;
			
            if(getline(ins[minIdx], line)) {
        	words[minIdx] = line;
            } else {
	        eofs[minIdx] = true;
            }
        }
		
    } while (minIdx != -1);

    std::cout << "Reduce done" << std::endl;
    out.close();

    for(size_t i = 0; i < 13; i++) {
	pthread_join(pids[i], NULL);
    }
    for (size_t i = 0; i < 13; i++) {
	ins[i].close();
    }
}

int main(int argc, char** argv)
{
    struct timespec ts;
    long long cpu_start_time;
    long long cpu_end_time;
    long long cpu_spent_time;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
    cpu_start_time = (ts.tv_sec)*1000 + (ts.tv_nsec/1000000);

    gettimeofday(&tv, NULL);
    start_time = (tv.tv_sec)*1000+(tv.tv_usec/1000);

    Globals = TaskFilter(argv[1]);
	
    gettimeofday(&tv, NULL);
    end_time = (tv.tv_sec)*1000+(tv.tv_usec/1000);
    clean_time = end_time - start_time;
    start_time = end_time;

    map3();

    gettimeofday(&tv, NULL);
    end_time = (tv.tv_sec)*1000+(tv.tv_usec/1000);
    map_time = end_time - start_time;
    start_time = end_time;

    reduce3(argv[2]);

    gettimeofday(&tv, NULL);
    end_time = (tv.tv_sec)*1000+(tv.tv_usec/1000);
    reduce_time = end_time - start_time;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
    cpu_end_time = (ts.tv_sec)*1000 + (ts.tv_nsec/1000000);
    cpu_spent_time = cpu_end_time - cpu_start_time;

    std::cout << "Clean time: " << to_string(clean_time) << std::endl;
    std::cout << "Map time: " << to_string(map_time) << std::endl;
    std::cout << "Reduce time: " << to_string(reduce_time) << std::endl;
    std::cout << "CPU time: " << to_string(cpu_spent_time) << std::endl;

    for (size_t i = 0; i < 13; i++) {
        //std::cout << to_string(i+3) << ":" << to_string((float)Indexs[i].size()/Globals.size()) << " ";
        std::cout << to_string(i+3) << ":" << to_string(Indexs[i].size()) << " ";
    }
    std::cout << std::endl;
    // std::cout << to_string(Globals.size()) << std::endl;
}
