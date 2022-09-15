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

void map4();
void reduce4(char *fn);

vector<string> Globals;
vector<vector<int>> Indexs(13);

pthread_t pids[13];
int       priority[13] = {0,0,0,0,0,0,0,0,0,0,0,0,0};

long long g_spent_time[13];
int w_count[13];

int cmp(const void *x, const void *y) {
    return strcmp(Globals[*(int*)x].substr(2).c_str(), Globals[*(int*)y].substr(2).c_str());
}

void* map4Task(void *argc) {

    clockid_t cid;
    struct timespec ts;
    long long cpu_start_time;
    long long cpu_end_time;
    long long cpu_spent_time;

    int taskid = *(int*)(argc);

    int s = pthread_getcpuclockid(pthread_self(), &cid);
    if (s != 0) {
        //std::cerr << "clockid error!" << std::endl;
    }
	
    //int res = nice(priority[taskid]);
    //if (res == -1) {
        //std::cerr << "Set priority error!" << std::endl;
    //}

    clock_gettime(cid, &ts);
    cpu_start_time = (ts.tv_sec)*1000 + (ts.tv_nsec/1000000);

    w_count[taskid] = Indexs[taskid].size();
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
    g_spent_time[taskid] = cpu_spent_time;

    //std::cerr << "pthread (" << to_string(taskid+3) << ") CPU time: " << to_string(cpu_spent_time) << std::endl;

    return (void*)NULL;
}

void map4(){
    struct sched_param sch_params;
    pthread_attr_t attr;

    pthread_attr_init(&attr);

    //std::cerr << "map4" << std::endl;
    // update 13 Indexs
	
    for(size_t i = 0; i < 13; i++) {
        Indexs[i].clear();
    }
    for(size_t i = 0; i < Globals.size(); i++) {
        Indexs[Globals[i].size()-3].push_back(i);
    }
	
    // create 13 map threads
    for(size_t i = 0; i < 13; i++) {
        int status = pthread_attr_getschedparam(&attr, &sch_params);
        sch_params.sched_priority = priority[i];
        status = pthread_attr_setschedpolicy(&attr, SCHED_RR);
        status = pthread_attr_setschedparam(&attr, &sch_params);
        if (status != 0) {
        }

        // create fifo
        string fifo = "fifo_" + to_string(i);
        mkfifo(fifo.c_str(), 0777);

        int *taskid = new int(i);
        pthread_create(&pids[i], &attr, map4Task, (void*)taskid);
    }
}

void reduce4(char *fn) {
    string result = string(fn);
	
    ifstream ins[13];
    bool     eofs[13];
    string   words[13];
    ofstream out;
    string   line;
	
    //std::cerr << "reduce4" << std::endl;
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

    int minIdx;
	
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

    //std::cerr << "Reduce done" << std::endl;
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
    struct timeval tv;
    long long start_time;
    long long end_time;

    long long clean_time;
    long long map_time;
    long long reduce_time;

    struct timespec ts;
    long long cpu_start_time;
    long long cpu_end_time;
    long long cpu_spent_time;

    string line;
    ofstream out;
    ifstream in;

    in.open("priority.txt");
    for (size_t i = 0; i < 13; i++) {
        if (getline(in, line)) {
            priority[i] = stoi(line);
        } else {
            priority[i] = 0;
        }
    }
    in.close();

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
    cpu_start_time = (ts.tv_sec)*1000 + (ts.tv_nsec/1000000);

    gettimeofday(&tv, NULL);
    start_time = (tv.tv_sec)*1000+(tv.tv_usec/1000);

    Globals = TaskFilter(argv[1]);

    gettimeofday(&tv, NULL);
    end_time = (tv.tv_sec)*1000+(tv.tv_usec/1000);
    clean_time = end_time - start_time;
    start_time = end_time;

    map4();

    gettimeofday(&tv, NULL);
    end_time = (tv.tv_sec)*1000+(tv.tv_usec/1000);
    map_time = end_time - start_time;
    start_time = end_time;

    reduce4(argv[2]);

    gettimeofday(&tv, NULL);
    end_time = (tv.tv_sec)*1000+(tv.tv_usec/1000);
    reduce_time = end_time - start_time;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
    cpu_end_time = (ts.tv_sec)*1000 + (ts.tv_nsec/1000000);
    cpu_spent_time = cpu_end_time - cpu_start_time;

    std::cerr << "Clean time: " << to_string(clean_time) << std::endl;
    std::cerr << "Map time: " << to_string(map_time) << std::endl;
    std::cerr << "Reduce time: " << to_string(reduce_time) << std::endl;
    std::cerr << "CPU time: " << to_string(cpu_spent_time) << std::endl;

    //for (int i = 0; i < 13; i++) {
    //    std::cout << to_string(g_spent_time[i]) << " ";
    //}
    //std::cout << std::endl;

    out.open("priority.txt");
    for (size_t i = 0; i < 13; i++) {
        out << to_string(priority[i]) << endl;
    }
    out.close();

    out.open("cpu_time.txt");
    for (size_t i = 0; i < 13; i++) {
        out << to_string(g_spent_time[i]) << endl;
    }
    out.close();
}
