#include "includes.h"
#include "mapThread.h"

using namespace std;

string firstWord;
string secondWord;

vector<mapThread> threadClass;
vector<thread> threadObj;
const int MAX_THREADS = 1;


int main()
{
    // initialize wordLoc to non valid values
    fill_n(wordLoc, 32, -1);

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
    string line;

    dictionary.open("dictionary.txt");

    if (!dictionary.is_open()) {
        printf("File not found\n");
    }
    else {
        while (getline(dictionary, line)) {
            dictionaryList.push_back(line);
        }
    }

    dictionary.close();
    cout << "File loaded\n";

    cout << "Press space to begin\n";
    _getch();
    system("CLS");

    string temp;

    // find smaller of the words
    if (firstWord.length() > secondWord.length()) {
        smallWord = secondWord;
        bigWord = firstWord;
    }
    else {
        smallWord = firstWord;
        bigWord = secondWord;
    }

    // set buffer sizes based off of word size
    if (smallWord.length() > 5) {
        smallBuffer = smallWord.length() / 3;
    }
    if (bigWord.length() > 7) {
        bigBuffer = bigWord.length() / 3;
    }

    cout << "Creating Threads\n";

    // get system clock time to calculate time elapsed later
    startTime = chrono::system_clock::now();

    // get first instance of word of target length
    mapTarget = binarySearchFirstLength(smallWord);

    for (int i = 0; i < MAX_THREADS; i++) {

        // create class instance
        mapThread temp;

        // add class instance to vector
        threadClass.push_back(temp);

        // add thread instance to vector and start it
        threadObj.push_back(thread(&mapThread::start, temp, i));
    }

    // binary time and info
    duration = chrono::system_clock::now() - startTime;
    cout << "Elapsed time (Binary search): " << (duration.count() * 1000.00) << "\n";
    cout << "Binary search return: " << mapTarget << endl;;

    // join threads after complete
    for (int i = 0; i < threadObj.size(); i++) {
        threadObj[i].join();
    }

    cout << "Threads joined\n";

    for (int i = 0; i < MAX_THREADS; i++) {
        // figure out c++ equivalent of putAll
        //EditNeighbors.insert(threadClass[i].getLocalMap());
    }

    duration = chrono::system_clock::now() - startTime;
    cout << "Elapsed time (Map creation): " << duration.count() << "s\n";
    cout << "Map size: " << threadClass[0].getLocalMap().size();

    return 0;
}