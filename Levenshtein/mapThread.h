#include "includes.h"

using namespace std;

class mapThread {
private:
    int threadID;
    string threadName;
public:

    mapThread(int threadID);
    void start();
    string getThreadName();
    int getThreadID();
  
};

mapThread::mapThread(int threadID) {
    this->threadID = threadID;
    threadName = "T-" + to_string(this->threadID);
}

void mapThread::start() {
    cout << threadName << ": Online\n";
}

string mapThread::getThreadName() {
    return threadName;
}

int mapThread::getThreadID() {
    return threadID;
}