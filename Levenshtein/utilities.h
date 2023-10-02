#include "includes.h"

using namespace std;

// Global methods--

bool isEditDistance(string in1, string in2) {
    int m = in1.length(), n = in2.length();

    int count = 0;

    int i = 0, j = 0;
    while (i < m && j < n)
    {
        if (in1[i] != in2[j]) {
            if (count == 1) {
                return false;
            }
            if (m > n) {
                i++;
            }
            else if (m < n) {
                j++;
            }
            else {
                i++;
                j++;
            }
            count++;
        }
        else {
            i++;
            j++;
        }
    }
    if (i < m || j < n) {
        count++;
    }
    return count == 1;
}

int getTarget()
{
    mapTarget++;
    return mapTarget - 1;
}

//--Global methods

// Global variables--

bool startThreads = false;
int mapTarget;

// make concurrent map later, need to get libraries for it
unordered_map <string, vector<string>> EditNeighbors;

//--Global variables