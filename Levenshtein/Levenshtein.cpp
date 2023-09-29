#include "includes.h"
#include "mapThread.h"

using namespace std;

string firstWord;
string secondWord;

vector<mapThread> threadClass;
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
    system("CLS");

    cout << "Creating Threads\n";

    // works
    //mapThread t1(1); thread t1Obj(&mapThread::start, t1, 1);
    

    for (int i = 0; i < MAX_THREADS; i++) {
       
        mapThread temp(i);

        threadClass.push_back(temp);

        threadObj.push_back(thread(&mapThread::start, temp));
    }

    cout << "Threads created\n";

    for (int i = 0; i < threadObj.size(); i++) {
        threadObj[i].join();
    }
}
