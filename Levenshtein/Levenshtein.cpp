#include "includes.h"
#include "mapThread.h"

using namespace std;

string firstWord;
string secondWord;

vector<mapThread> threadClass;
vector<thread> threadObj;
const int MAX_THREADS = 16;


int main()
{

    // initialize wordLoc to non valid valuess
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

    // get system clock time to calculate time elapsed later
    start = chrono::system_clock::now();
    chrono::duration<double> duration;

    string temp;

    // find smaller of the words
    if (firstWord.length() > secondWord.length()) {
        smallWord = secondWord;
        bigWord = firstWord;
    } else {
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

    // get first instance of word of target length
    mapTarget = binarySearchFirstLength(smallWord);
    
    // binary time and info
    duration = chrono::system_clock::now() - start;
    cout << "Elapsed time (Binary search): " << (duration.count() * 1000.00) << "ms\n";
    cout << "Binary search return: " << mapTarget << endl;

    startThreads = true;

    for (int i = 0; i < threadObj.size(); i++) {
        threadObj[i].join();
    }

    cout << "Threads joined\n";

    for (int i = 0; i < MAX_THREADS; i++) {
        // figure out c++ equivalent of putAll
        //EditNeighbors.insert(threadClass[i].getLocalMap());
    }

    duration = chrono::system_clock::now() - start;

    cout << "Elapsed time (Map creation): " << duration.count() << "s\n";

    return 0;
}
