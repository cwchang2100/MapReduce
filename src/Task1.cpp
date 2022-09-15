#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <random>
#include <time.h>
#include <sys/time.h>
using namespace std;

extern vector<string>TaskFilter(string fn1);

extern long long filter_start_time;
extern long long filter_end_time;
extern long long filter_spent_time;

struct timeval start_tv;
struct timeval filter_tv;
struct timeval end_tv;
long long start_time;
long long filter_time;
long long end_time;

long long clean_time;
long long output_time;

vector<string> Globals;

int main(int argc, char** argv)
{
    ofstream out;
    //clockid_t cid;
    struct timespec ts;
    long long cpu_start_time;
    long long cpu_end_time;
    long long cpu_spent_time;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
    cpu_start_time = (ts.tv_sec)*1000 + (ts.tv_nsec/1000000);

    gettimeofday(&start_tv, NULL);
    start_time = (start_tv.tv_sec)*1000 + (start_tv.tv_usec/1000);

    Globals = TaskFilter(argv[1]);

    gettimeofday(&filter_tv, NULL);
    filter_time = (filter_tv.tv_sec)*1000 + (filter_tv.tv_usec/1000);

    out.open(argv[2]);
    for(size_t i = 0; i<Globals.size();i++) {
        out << Globals[i] << endl;
    }
    out.close();

    gettimeofday(&end_tv, NULL);
    end_time = (end_tv.tv_sec)*1000 + (end_tv.tv_usec/1000);
    clean_time  = filter_time - start_time;
    output_time = end_time    - filter_time;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
    cpu_end_time = (ts.tv_sec)*1000 + (ts.tv_nsec/1000000);
    cpu_spent_time = cpu_end_time - cpu_start_time;

    std::cerr << "Clean time: " << to_string(clean_time) << std::endl;
    std::cerr << "Output time: " << to_string(output_time) << std::endl;
    std::cerr << "CPU time: " << to_string(cpu_spent_time) << std::endl;
    std::cerr << "written to the clean file:" << argv[2] << std::endl;
}
