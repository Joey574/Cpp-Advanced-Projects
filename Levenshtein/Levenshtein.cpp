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

    for (int i = 0; i < MAX_THREADS; i++) {
       
        // create class instance
        mapThread temp(i);

        // add class instance to vector
        threadClass.push_back(temp);

        // add thread instance to vector and start it
        threadObj.push_back(thread(&mapThread::start, temp));
    }

    cout << "Threads created\nPress space to begin";
    
    _getch();

    for (int i = 0; i < threadObj.size(); i++) {
        threadObj[i].join();
    }
}
