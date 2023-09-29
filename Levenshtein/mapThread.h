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
    gotoxy(threadID * 10, threadID * 5);
    cout << threadName << ": Online\n";
}

string mapThread::getThreadName() {
    return threadName;
}

int mapThread::getThreadID() {
    return threadID;
}