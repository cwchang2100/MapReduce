#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <random>
#include <time.h>
#include <sys/time.h>
using namespace std;

pthread_t pids[13];
int       priority[13] = {0,0,0,0,0,0,0,0,0,0,0,0,0};

long long g_spent_time[13];
//int w_count[13];

int priority_adjust() {

    long long average;
    int adjusted = 0;

    average = 0;
    for (int i = 0; i < 13; i++) {
        average += g_spent_time[i];
    }

    average = average/13;

    adjusted = 0;

    for (int i = 0; i < 13; i++) {

        int diff = g_spent_time[i] - average;

        if (diff > average/5) { // slow
            if (priority[i] > -20) { // faster
                priority[i]--;
                adjusted = 1;
            }
        } else if (diff < -(average/5)) { // fast
            if (priority[i] < 19) { // slower
                priority[i]++;
                adjusted = 1;
            }
        }
    }
    return adjusted;
}


int main(int argc, char** argv)
{
    int res = 0;
    do {
        char cmd[256];

        sprintf(cmd, "%s %s %s", argv[1], argv[2], argv[3]);
        if (system(cmd) != 0) {
            cerr << "system failed!" << endl;
        }

        string line;
        ifstream in;

        in.open("cpu_time.txt");
        for (size_t i = 0; i < 13; i++) {
            if (getline(in, line)) {
                g_spent_time[i] = stoi(line);
            } else {
                g_spent_time[i] = 0;
            }
        }
        in.close();

        res = priority_adjust();

        ofstream out;
        out.open("priority.txt");
        for (size_t i = 0; i < 13; i++) {
            out << to_string(priority[i]) << endl;
        }
        out.close();

        for (int i = 0; i < 13; i++) {
            std::cerr << to_string(priority[i]) << " ";
        }
        std::cerr << std::endl;

        for (int i = 0; i < 13; i++) {
            std::cout << to_string(g_spent_time[i]) << " ";
        }
        std::cout << std::endl;

    } while (res == 1);
}
