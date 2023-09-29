#include "includes.h"
#include "mapThread.h"

using namespace std;

string firstWord;
string secondWord;

vector<mapThread> threads;
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

    int test = 5;
     
    /*
    for (int i = 0; i < MAX_THREADS; i++) {
        mapThread temp(i);
        cout << "Thread " << i << " active";
        thread t(temp);
        cout << temp.getName() + " active";
    }
    */

    mapThread t;

    thread thread_object(&mapThread::start, t, 1);

}
