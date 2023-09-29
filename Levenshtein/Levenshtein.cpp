#include "includes.h"
#include "mapThread.h"

using namespace std;

string firstWord;
string secondWord;

vector<mapThread> threads;
vector<thread> threadObj;
int MAX_THREADS = 16;

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
        mapThread temp(i);
        cout << "Thread " << i << " created";
        threads.push_back(temp);
        thread thread_obj(&mapThread::start, &temp, i);
        threadObj.push_back(thread_obj);
        cout << temp.getThreadName() + " active";
    }

    for (thread t : threadObj) {
        t.join();
    }

}
