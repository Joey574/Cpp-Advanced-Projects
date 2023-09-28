#include <iostream>
#include <thread>
#include <conio.h>

using namespace std;

string firstWord;
string secondWord;

int MAX_THREADS = 16;

class mapThread {
private:
    int threadID;
    string threadName;


    mapThread(int threadID) {
        this->threadID = threadID;
        threadName = "\nT-" + threadID;
        cout << threadName + " running";
    }
};

int main()
{
    cout << "This is the basic multi-threaded Levenshtein in Cpp\n";
    _getch();
    system("CLS");
    cout << "Enter the first word\nInput: ";
    cin >> firstWord;
    system("CLS");
    cout << "Enter the second word\nInput: ";
    cin >> secondWord;

    for (int i = 0; i < MAX_THREADS; i++) {
        cout << "Thread " + i + " created";
        thread thread(mapThread(i));
    }


}
