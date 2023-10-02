#include "includes.h"

using namespace std;

class mapThread {
private:
    int threadID;
    string threadName;
public:

    mapThread(int threadID);

    void start();

    // Getters --

    string getThreadName();
    int getThreadID();

    // -- Getters  
};

mapThread::mapThread(int threadID) {
    this->threadID = threadID;
    threadName = "T-" + to_string(this->threadID);
}

void mapThread::start() {
    string out = threadName + ": Online\n";
    cout << out;

    while (startThreads) {
        Sleep(1);
    }
}

string mapThread::getThreadName() {
    return threadName;
}

int mapThread::getThreadID() {
    return threadID;
}