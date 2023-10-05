#include "includes.h"

using namespace std;

class mapThread {
private:
    int threadID;
    string threadName;
    string out;

    unordered_map<string, vector<string>> EditNeighborsLoc;
public:

    void start(int threadID);

    // Getters --

    unordered_map<string, vector<string>> getLocalMap();
    string getThreadName();
    int getThreadID();

    // -- Getters  
};

string mapThread::getThreadName() {
    return threadName;
}

int mapThread::getThreadID() {
    return threadID;
}

unordered_map<string, vector<string>> mapThread::getLocalMap() {
    return EditNeighborsLoc;
}

void mapThread::start(int threadID) {

    unordered_map<string, vector<string>> EditNeighborsLoc;

    this->threadID = threadID;
    threadName = "T-" + to_string(threadID);
    out = threadName + ": Online\n";
    cout << out;

    bool complete = false;
    vector<string> neighbors;
    string word;

    for (int i = 0; word.length() <= bigWord.length() + bigBuffer; i++) {

        int target = getTarget();
        word = dictionaryList[target];

        neighbors.clear();

        for (int q = binarySearchFirstLength(word.substr(1)); q < dictionaryList.size(); q++) {

            string temp = dictionaryList[q];
            if (temp.length() > word.length() + 1) {
                break;
            } else if (isEditDistance(temp, word)) {
                neighbors.push_back(temp);
            }
        }
        EditNeighborsLoc[word] = neighbors;
    }

    cout << "local size: " << EditNeighborsLoc.size() << endl;
    out = threadName + ": Complete\n";
    cout << out;
}

