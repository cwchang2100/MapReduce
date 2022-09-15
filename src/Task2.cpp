#include <iostream>
#include <string>
#include <fstream>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <random>
#include <thread>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

using namespace std;

extern vector<string> TaskFilter(string fn1);

vector<string> Globals;
vector<vector<int>> Indexs(13);
size_t heads[13];
string fns[13];

void reduce(string file);

pid_t pids[13];

int   fds[13];
off_t size[13];
off_t pa_size[13];
char *shm_addr[13];

struct timeval tv;

long long start_time;
long long end_time;

long long clean_time;
long long map_time;
long long reduce_time;

int cmp(const void *x, const void *y) {
    return strcmp(Globals[*(int*)x].substr(2).c_str(), Globals[*(int*)y].substr(2).c_str());
}


void map2(){

    ifstream in;

    for (size_t j = 0; j < Globals.size(); j++) {
        Indexs[Globals[j].size()-3].push_back(j);
    }
    std::cout << "map2 done" << std::endl;
    
    for(size_t i = 3; i <= 15; i++) {
		
        int pid;

        pids[i-3] = 0;

        size[i-3]     = Indexs[i-3].size()*sizeof(int);
        pa_size[i-3]  = (size[i-3] + sysconf(_SC_PAGE_SIZE)) & ~(sysconf(_SC_PAGE_SIZE) - 1);
        shm_addr[i-3] = (char *)mmap(NULL, pa_size[i-3], PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);

	pid = fork();
	if (pid == 0) {

            struct timespec ts;
            long long cpu_start_time;
            long long cpu_end_time;
            long long cpu_spent_time;

            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
            cpu_start_time = (ts.tv_sec)*1000 + (ts.tv_nsec/1000000);

            qsort(&Indexs[i-3][0], Indexs[i-3].size(), sizeof(int), cmp);

            memcpy(shm_addr[i-3], &Indexs[i-3][0], size[i-3]);
            msync(shm_addr[i-3], pa_size[i-3], MS_SYNC);

            ofstream out;
            out.open("sorted"+to_string(i)+".txt");
            for (size_t j = 0; j < Indexs[i-3].size(); j++) {
                out << Globals[Indexs[i-3][j]] << std::endl;
            }
            out.close();

            heads[i-3] = 0;

            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
            cpu_end_time = (ts.tv_sec)*1000 + (ts.tv_nsec/1000000);
            cpu_spent_time = cpu_end_time - cpu_start_time;

            std::cout << "fork (" << to_string(i) << ") CPU time: " << to_string(cpu_spent_time) << std::endl;

	    exit(0);
	} if (pid > 0) {
            pids[i-3] = pid;
        }
    }
}

void reduce(string result) {
    ifstream ins[13];
    bool     eofs[13];
    string   words[13];
    ofstream out;
    string   line;
	
    std::cout << "reduce start" << std::endl;
    out.open(result);
	
    for(size_t i = 3; i <= 15; i++) {
        int   status;
        pid_t pid;

        while ((pid = waitpid(pids[i-3], &status, WNOHANG)) == 0) {
            //std::cout << "Child is still running!" << std::endl;
        }

        memcpy(&Indexs[i-3][0], shm_addr[i-3], size[i-3]);

        heads[i-3] = 0;
        if (Indexs[i-3].size() > 0) {
	    words[i-3] = Globals[Indexs[i-3][0]];
            heads[i-3]++;
	    eofs[i-3]  = false;
        } else {
	    words[i-3] = "";
	    eofs[i-3]  = true;
        }
    }

    int minIdx = -1;
	
    do {
	string minStr = "";
        minIdx = -1;
		
	for(size_t i = 0; i < 13; i++) {
	    if (eofs[i] == false) {
		if (minIdx == -1 || words[i].substr(2) < minStr) {
		    minIdx = i;
		    minStr = words[i].substr(2);
	        }
	    }
	}
		
	if (minIdx != -1) {

            out << words[minIdx] << std::endl;

            if (heads[minIdx] < Indexs[minIdx].size()) { // more in minIdx
	        if (Indexs[minIdx][heads[minIdx]] > (int)Globals.size() || Indexs[minIdx][heads[minIdx]] < 0) {
                    std::cout << "ERROR " << to_string(minIdx) << " " << to_string(heads[minIdx]) << std::endl;
                    std::cout << "Value " << to_string(Indexs[minIdx][heads[minIdx]]) << std::endl;
                }
	        words[minIdx] = Globals[Indexs[minIdx][heads[minIdx]]];
                heads[minIdx]++;
            } else {
	        eofs[minIdx] = true;
            }
	}
		
    } while (minIdx != -1);

    for (size_t i = 3; i <= 15; i++) {
        munmap(shm_addr[i-3], pa_size[i-3]);
    }
    std::cout << "reduce end" << std::endl;

    out.close();
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

    map2();

    gettimeofday(&tv, NULL);
    end_time = (tv.tv_sec)*1000+(tv.tv_usec/1000);
    map_time = end_time - start_time;
    start_time = end_time;

    reduce(argv[2]);

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
}
