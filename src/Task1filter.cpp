#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <random>
#include <sys/time.h>

using namespace std;

#define ALPHA_DIGIT 1

static struct timeval start_tv;
static struct timeval end_tv;
long long filter_start_time;
long long filter_end_time;
long long filter_spent_time;

vector<string> TaskFilter(string dirtyFile) {
	
    vector<string> words;
    ifstream in;
    in.open(dirtyFile);
    std::string line;
    std::string cleanline;

    gettimeofday(&start_tv, NULL);
    filter_start_time = (start_tv.tv_sec)*1000+(start_tv.tv_usec/1000);

    // filter words 
    while (getline(in, line))
    {
	cleanline = "";
	for (size_t i = 0; i < line.size(); i++)  {
#ifdef ALPHA_DIGIT
            if (((line[i] >= 'A') && (line[i] <= 'Z')) ||
                ((line[i] >= 'a') && (line[i] <= 'z')) ||
                ((line[i] >= '0') && (line[i] <= '9'))) {  
#else
            if (((line[i] >= 'A') && (line[i] <= 'Z')) ||
                ((line[i] >= 'a') && (line[i] <= 'z'))) {
#endif
	        cleanline += line[i]; 
            }
        }
	if (cleanline.size() >= 3 && cleanline.size() <= 15) {
            words.push_back(cleanline);
	}
    }
    in.close();

    // remove duplicates
    std::sort(words.begin(),words.end());
	
    words.erase(std::unique(words.begin(), words.end()), words.end());

    //std::mt19937 random(std::random_device{}());
    //shuffle(words.begin(),words.end(),random);

    gettimeofday(&end_tv, NULL);
    filter_end_time = (end_tv.tv_sec)*1000+(end_tv.tv_usec/1000);
    filter_spent_time = filter_end_time - filter_start_time;

    return words;
}
