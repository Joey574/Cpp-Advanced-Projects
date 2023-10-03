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

    cout << "Loading file\n";

    ifstream dictionary;

    dictionary.open("dictionary.txt");

    if (!dictionary.is_open())
    {
        printf("File not found\n");
    }

    string line;

    while (getline(dictionary, line)) {
        dictionaryList.push_back(line);
    }
    
    dictionary.close();
    cout << "File loaded\n";

    cout << "Creating Threads\n";

    for (int i = 0; i < MAX_THREADS; i++) {
       
        // create class instance
        mapThread temp(i);

        // add class instance to vector
        threadClass.push_back(temp);

        // add thread instance to vector and start it
        threadObj.push_back(thread(&mapThread::start, temp));
    }

    cout << "Threads created\nPress space to begin\n";

    _getch();
    system("CLS");

    start = chrono::system_clock::now();
    chrono::duration<double> duration;


    string smallWord;
    string bigWord;

    string temp;

    if (firstWord > secondWord) {
        smallWord = secondWord;
        bigWord = firstWord;
    } else {
        smallWord = firstWord;
        bigWord = secondWord;
    }

    mapTarget = binarySearchFirstLength(smallWord);
    duration = chrono::system_clock::now() - start;
    cout << "Elapsed time (Binary search): " << duration.count() << "s\n";
    cout << "Binary search return: " << mapTarget << endl;

    startThreads = true;

    for (int i = 0; i < threadObj.size(); i++) {
        threadObj[i].join();
    }

    cout << "Threads joined\n";

    duration = chrono::system_clock::now() - start;

    cout << "Elapsed time (Map creation): " << duration.count() << "s\n";

    return 0;
}
