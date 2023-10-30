#pragma once
#include <stdio.h>
#include <iostream>
#include <string> 
#include <stdlib.h> 
#include <time.h> 
#include <conio.h> 
#include <vector>
#include <windows.h> 
#include <thread>
#include <math.h>
#include <sstream> 
#include <fstream>
#include <cwchar>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <map>
#include <concurrent_unordered_map.h>

using namespace std;

bool isEditDistance(string in1, string in2);
int binarySearchFirstLength(string word);
vector<string> getNeighbors(string word);

chrono::system_clock::time_point startTime;
chrono::duration<double> duration;
int wordLoc[32] = { 0 };

vector<string> dictionaryList;

int main()
{
    string firstWord;
    string secondWord;

    // initialize wordLoc to non valid values
    fill_n(wordLoc, 32, -1);

    cout << "This is the basic graphing Levenshtein in Cpp\n";
    _getch();
    system("CLS");
    cout << "Enter the first word\nInput: ";
    cin >> firstWord;
    system("CLS");
    cout << "Enter the second word\nInput: ";
    cin >> secondWord;
    system("CLS");

    cout << "Loading file\n";

    ifstream dictionary;
    string line;

    dictionary.open("dictionary.txt");

    if (!dictionary.is_open()) {
        printf("File not found\n");
    }
    else {
        while (getline(dictionary, line)) {
            dictionaryList.push_back(line);
        }
    }

    dictionary.close();
    cout << "File loaded\n";

    cout << "Press space to begin\n";
    _getch();
    system("CLS");

    startTime = chrono::system_clock::now();

    unordered_set <string> path;
    vector<string> values;
    vector<string> frontier;
    string word = firstWord;
    string temp;

    bool complete = false;

    /*
    BFS
    add current to path
    add neighbors to frontier
    grab location from queue
    check if in path
    if path repeat
    check if final
    repeat
    */

    while (!complete) {
        path.insert(word); // add curent to path
        values = getNeighbors(word); // get neighbors
        
        for (string x : values) { // add neighbors to frontier
            frontier.push_back(x);
        }

        for (int i = 0; i < frontier.size(); i++) {
            if (!path.contains(frontier[i])) { // check if in path
                word = frontier[i];
                break;
            }
            else { // if in path repeat
                values.erase(values.begin());
            }
        }

        if (word == secondWord) {
            complete;
        }

    }

    for (string p : path) {
        cout << p << endl;
    }

    
    /*
    DFS

    add current to path
    add neighbors to frontier
    grab location from stack
    check if in path
    if path repeat
    check if final
    repeat

    GFS

    add current to path
    add neighbors to frontier
    find best based on edit dist
    check if in path
    if path repeat
    check if final
    repeat


    */
   
}

vector<string> getNeighbors(string word) {

    bool complete = false;
    vector<string> neighbors;
    string wordT;

    for (int q = binarySearchFirstLength(wordT.substr(1)); q < dictionaryList.size(); q++) {

        string temp = dictionaryList[q];
        if (temp.length() > wordT.length() + 1) {
            break;
        }
        else if (isEditDistance(temp, wordT)) {
            neighbors.push_back(temp);
        }
    }
    
    return neighbors;
}

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
        }
        else if (dictionaryList[loc].length() > len) {
            max = loc;
        }
        else {
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