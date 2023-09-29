#include "includes.h"

using namespace std;

class mapThread {
private:
    int threadID;
    string threadName;


public:

    void start();
    string getThreadName();
    int getThreadID();
  
};

void mapThread::start() {
    this->threadID = threadID;
    threadName = "\nT-" + this->threadID;
}

string mapThread::getThreadName() {
    return threadName;
}

int mapThread::getThreadID() {
    return threadID;
}