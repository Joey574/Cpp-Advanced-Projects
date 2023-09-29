#include "includes.h"

class mapThread {
private:
    int threadID;
    string threadName;


public:

    void start();
    string getThreadName();
  
};

void mapThread::start() {
    this->threadID = threadID;
    threadName = "\nT-" + this->threadID;
}