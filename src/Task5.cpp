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

vector<string> DirtyWords;

vector<vector<string>> Words(13);

pthread_t pids[13];
int       priority[13] = {0,0,0,0,0,0,0,0,0,0,0,0,0};

long long g_spent_time[13];
int w_count[13];

void* DirtyServer(void *dirtyFile) {
	
    string dirtyfilename((char *)dirtyFile);
    ifstream in;
    in.open(dirtyfilename);
    std::string line;
    std::string cleanline;

    std::cout << "DirtyServer start... " << std::endl;

    while (getline(in, line))
    {
       DirtyWords.push_back(line);
    }
    in.close();
    std::cout << "Dirty file Read done" << std::endl;

    //std::mt19937 random(std::random_device{}());
    //std::shuffle(DirtyWords.begin(),DirtyWords.end(),random);

    string fifo = "fifo_dirty";
    ofstream of;
    of.open(fifo);

    for (size_t i = 0; i < DirtyWords.size(); i++) {
	line = DirtyWords[i];
	cleanline = "";

	for (size_t i = 0; i < line.size(); i++)  { // filter out
            if (((line[i] >= 'A') && (line[i] <= 'Z')) ||
                ((line[i] >= 'a') && (line[i] <= 'z')) ||
		((line[i] >= '0') && (line[i] <= '9'))) {  
		cleanline += line[i]; 
            }
        }
        if (cleanline.size() >= 3 && cleanline.size() <= 15) {
            of << cleanline << std::endl; // output to pipeline
        }
    }
    of.close();
    std::cout << "DirtyServer end... " << std::endl;

    return (void*)NULL;
}

int cmp(const void *e1, const void *e2) {
    string *s1 = (string *)e1;
    string *s2 = (string *)e2;
    if (*s1 < *s2)      return -1;
    else if(*s1 == *s2) return 0;
    else if(*s1 >  *s2) return 1;
    return 0;
}

void* map5Task(void *argc) {
    struct timeval tv;
    long long start_time;
    long long end_time;

    clockid_t cid;
    struct timespec ts;
    long long cpu_start_time;
    long long cpu_end_time;
    long long cpu_spent_time;

    int taskid = *(int*)(argc);
	
    int s = pthread_getcpuclockid(pthread_self(), &cid);
    if (s != 0) {
        std::cout << "clockid error!" << std::endl;
    }

    int res = nice(priority[taskid]);
    if (res == -1) {
        std::cout << "Set priority error !" << std::endl;
    }

    clock_gettime(cid, &ts);
    cpu_start_time = (ts.tv_sec)*1000 + (ts.tv_nsec/1000000);

    gettimeofday(&tv, NULL);
    start_time = (tv.tv_sec)*1000+(tv.tv_usec)/1000;

    // qsort 
    // qsort(&Words[taskid][0], Words[taskid].size(), sizeof(string), cmp);
    std::sort(Words[taskid].begin(),Words[taskid].end());	
    Words[taskid].erase(std::unique(Words[taskid].begin(), Words[taskid].end()), Words[taskid].end());
    w_count[taskid] = Words[taskid].size();

    // write fifo
    string fifo = "fifo_" + to_string(taskid);
    ofstream of;
    of.open(fifo);

    for(size_t i = 0; i < Words[taskid].size(); i++) {
        of << Words[taskid][i] << std::endl;
    }
    of.close();
	
    delete (int*)argc;
	
    gettimeofday(&tv, NULL);
    end_time = (tv.tv_sec)*1000+(tv.tv_usec)/1000;
    g_spent_time[taskid] = end_time - start_time;

    //std::cout << "pthread (" << to_string(taskid+3) << ") " << to_string(w_count[taskid]) << " " << to_string(g_spent_time[taskid]) << std::endl;

    clock_gettime(cid, &ts);
    cpu_end_time = (ts.tv_sec)*1000 + (ts.tv_nsec/1000000);
    cpu_spent_time = cpu_end_time - cpu_start_time;

    std::cout << "pthread (" << to_string(taskid+3) << ") CPU time: " << to_string(cpu_spent_time) << std::endl;

    return (void*)NULL;
}

void map5(){
    string line;
    std::cout << "map5" << std::endl;

    ifstream in;
    string fifo = "fifo_dirty";

    in.open(fifo.c_str());
    while (getline(in, line)) {
        //std::cout << to_string(line.size()) << std::endl;
        Words[line.size()-3].push_back(line);
    }
    in.close();
	
    for(size_t i = 0; i < 13; i++) {

        //std::cout << to_string(Words[i].size()) << std::endl;
        // create fifo
        string fifo = "fifo_" + to_string(i);
        mkfifo(fifo.c_str(), 0777);

        int *taskid = new int(i);
        pthread_create(&pids[i], NULL, map5Task, (void*)taskid);
    }
}

void reduce5(char *fn) {
    string result = string(fn);
	
    ifstream ins[13];
    bool     eofs[13];
    string   words[13];
    ofstream out;
    string   line;
	
    std::cout << "reduce5" << std::endl;
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
            if (eofs[i] == false) {
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

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
    cpu_start_time = (ts.tv_sec)*1000 + (ts.tv_nsec/1000000);

    gettimeofday(&tv, NULL);
    start_time = (tv.tv_sec)*1000+(tv.tv_usec/1000);

    mkfifo("fifo_dirty", 0777);

    pthread_t pid;
    pthread_create(&pid, NULL, DirtyServer, (void*)argv[1]);

    gettimeofday(&tv, NULL);
    end_time = (tv.tv_sec)*1000+(tv.tv_usec/1000);
    clean_time = end_time - start_time;
    start_time = end_time;

    map5();

    gettimeofday(&tv, NULL);
    end_time = (tv.tv_sec)*1000+(tv.tv_usec/1000);
    map_time = end_time - start_time;
    start_time = end_time;

    reduce5(argv[2]);
	
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

    pthread_join(pid, NULL);
}
