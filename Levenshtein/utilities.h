#include "includes.h"

using namespace std;


// Global variables--

int wordLoc[32] = { 0 };
atomic_int mapTarget;

int smallBuffer;
int bigBuffer;

chrono::system_clock::time_point startTime;
chrono::duration<double> duration;


string smallWord;
string bigWord;

vector<string> dictionaryList;

// make concurrent map later, need to get libraries for it
unordered_map <string, vector<string>> EditNeighbors;

//--Global variables

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
    return mapTarget.fetch_add(1);
}

int binarySearchFirstLength(string word) {

    int len = word.length();

    // memorization, check if already calculated
    if (wordLoc[len] != -1) {
        return wordLoc[len];
    }

    int min = 0;
    int max = dictionaryList.size();
    int loc = (max + min) / 2;

    bool complete = false;
    int out = -1;

    // binary search
    for (int i = 0; !complete; i++) {
        loc = (max + min) / 2;

        if (dictionaryList[loc].length() == len) {
            complete = true;
            out = loc;
        } else if (dictionaryList[loc].length() > len) {
            max = loc;
        } else {
            min = loc;
        }

        if (min >= max) {
            complete = true;
        }
    }

    // find first instance of word of target length
    for (int i = out; dictionaryList[out].length() == len; i++) {
        out--;
    }
    out++; // fix offset

    wordLoc[len] = out;
    return out;
}

//--Global methods