#include <vector>
#include <iostream>
#include <string>
#include <chrono>
#include <Windows.h>

#define RED_TEXT 4
#define GREEN_TEXT 10
#define WHITE_TEXT 7
#define BLUE_TEXT 1
#define YELLOW_TEXT 6
#define PURPLE_TEXT 13

void run_test(std::vector<int>(*sorter)(std::vector<int>), std::string name);
std::vector<int> selecition_sort(std::vector<int> unsorted);
std::vector<int> insertion_sort(std::vector<int> unsorted);
std::vector<int> bubble_sort(std::vector<int> unsorted);
std::vector<int> merge_sort(std::vector<int> unsorted);
std::vector<int> comb_sort(std::vector<int> unsroted);
std::vector<int> tim_sort(std::vector<int> unsorted);

std::vector<int> bogo_sort(std::vector<int> unsorted);
std::vector<int> bogo_seed_find(std::vector<int> unsorted);

int main()
{
    run_test(selecition_sort, "selection sort");
    run_test(insertion_sort, "insertion sort");
    run_test(bubble_sort, "bubble sort");
    run_test(merge_sort, "merge sort");
    //run_test(comb_sort, "comb sort"); -> needs to be fixed
    //run_test(tim_sort, "tim sort"); -> needs to be made

    //run_test(bogo_sort, "bogo sort");
    //run_test(bogo_seed_find, "seed find: ");
}

void run_test(std::vector<int>(*sorter)(std::vector<int>), std::string name) {

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, YELLOW_TEXT); std::cout << name << ":\n";

    const int max_size = 8192;
    const int multiplier = 2;
    const int runs = 10;

    srand(0);

    for (int size = 1024; size <= max_size; size *= multiplier) {
        std::vector<int> to_sort(size);

        // init values to 0->size - 1
        for (int i = 0; i < to_sort.size(); i++) {
            to_sort[i] = i;
        }

        // shuffle
        for (int k = 0; k < to_sort.size(); k++) {
            int r = k + rand() % (to_sort.size() - k);

            int t = to_sort[k];
            to_sort[k] = to_sort[r];
            to_sort[r] = t;
        }

        double best[runs];

        for (int r = 0; r < runs; r++) {
            auto start = std::chrono::high_resolution_clock::now();
            std::vector<int> sorted = sorter(to_sort);
            best[r] = (std::chrono::high_resolution_clock::now() - start).count();

            for (int i = 0; i < sorted.size(); i++) {
                if (sorted[i] != i) {
                    std::cout << name << " failed\n";
                    break;
                }
            }

        }
        
        double min = *std::min_element(&best[0], &best[runs]);
        double max = *std::max_element(&best[0], &best[runs]);

        SetConsoleTextAttribute(hConsole, WHITE_TEXT); std::cout << "\t" << size << ": ";
        SetConsoleTextAttribute(hConsole, GREEN_TEXT); std::cout << (min / 1000000.00) << "ms";
        SetConsoleTextAttribute(hConsole, WHITE_TEXT); std::cout << " - ";
        SetConsoleTextAttribute(hConsole, RED_TEXT); std::cout << (max / 1000000.00) << "ms\n";

        SetConsoleTextAttribute(hConsole, WHITE_TEXT);
    }
}

std::vector<int> selecition_sort(std::vector<int> unsorted) {
    for (int i = 0; i < unsorted.size() - 1; i++) {

        int min = unsorted[i];
        int min_idx = i;

        for (int j = i + 1; j < unsorted.size(); j++) {
            if (unsorted[j] < min) {
                min = unsorted[j];
                min_idx = j;
            }
        }
        
        int t = unsorted[i];
        unsorted[i] = min;
        unsorted[min_idx] = t;
    }
    return unsorted;
}
std::vector<int> insertion_sort(std::vector<int> unsorted) {
    for (int i = 1; i < unsorted.size(); i++) {
        int val = unsorted[i];
        int j = i - 1;

        while (j >= 0 && unsorted[j] > val) {
            unsorted[j + 1] = unsorted[j];
            j--;
        }
        unsorted[j + 1] = val;
    }

    return unsorted;
}
std::vector<int> bubble_sort(std::vector<int> unsorted) {
    for (int i = 0; i < unsorted.size() - 1; i++) {
        for (int j = 0; j < unsorted.size() - 1; j++) {
            if (unsorted[j] > unsorted[j + 1]) {
                std::swap(unsorted[j + 1], unsorted[j]);
            }
        }
    }

    return unsorted;
}
std::vector<int> merge_sort(std::vector<int> unsorted) {

    if (unsorted.size() < 2) {
        return unsorted;
    }

    std::vector<int> a = merge_sort(std::vector<int>(unsorted.begin(), unsorted.begin() + (unsorted.size() / 2)));
    std::vector<int> b = merge_sort(std::vector<int>(unsorted.begin() + (unsorted.size() / 2), unsorted.end()));

    int a_idx = 0;
    int b_idx = 0;
    for (int i = 0; i < unsorted.size(); i++) {

        if (a_idx >= a.size()) {
            // append b if we exhaust a
            std::copy(&b[b_idx], &b.back() + 1, &unsorted[i]);
            break;
        } else if (b_idx >= b.size()) {
            // append a if we exhaust b
            std::copy(&a[a_idx], &a.back() + 1, &unsorted[i]);
            break;
        } else {
            // set next element to min of a and b
            if (a[a_idx] < b[b_idx]) {
                unsorted[i] = a[a_idx];
                a_idx++;
            } else {
                unsorted[i] = b[b_idx];
                b_idx++;
            }
        }
    }

    return unsorted;
}
std::vector<int> comb_sort(std::vector<int> unsorted) {

    // init gap to size of vector
    int gap = unsorted.size();
    bool swap = true;

    while (gap > 0 && swap) {

        // scale gap by 10/13
        gap = (int)(gap * (10.0f / 13.0f));

        swap = false;
        for (int i = 0; i < unsorted.size() - gap; i++) {
            if (unsorted[i] > unsorted[i + gap]) {
                int t = unsorted[i];
                unsorted[i] = unsorted[i + gap];
                unsorted[i + gap] = t;
                swap = true;
            }
        }
    }

    return unsorted;
}
std::vector<int> tim_sort(std::vector<int> unsorted) {
    int run_size = 32;

    for (int i = 0; i < unsorted.size(); i += run_size) {

        // insertion sort over runs
        for (int j = i + 1; j < unsorted.size() && j < i + run_size; j++) {
            int val = unsorted[j];
            int k = i - 1;

            while (k >= 0 && unsorted[k] > val) {
                unsorted[k + 1] = unsorted[k];
                k--;
            }
            unsorted[k + 1] = val;
        }
    }

    for (int i = 0; i < unsorted.size(); i += run_size, run_size *= 2) {



    }

    /*
    * use insertion sort over runs
    * 
    * merge runs using modified merge sort
    * 
    * double run size
    * 
    * loop {
    *   merge runs
    * }
    * 
    */

    for (int i = 0; i < unsorted.size(); i++) {
        std::cout << unsorted[i] << " ";
    }
    return unsorted;
}
 
std::vector<int> bogo_sort(std::vector<int> unsorted) {
start:
    for (int k = 0; k < unsorted.size(); k++) {
        int r = k + rand() % (unsorted.size() - k);

        int t = unsorted[k];
        unsorted[k] = unsorted[r];
        unsorted[r] = t;
    }

    for (int i = 0; i < unsorted.size() - 1; i++) {
        if (unsorted[i] > unsorted[i + 1]) {
            goto start;
        }
    }
    return unsorted;
}
std::vector<int> bogo_seed_find(std::vector<int> unsorted) {

    int seed = 0;
start:
    srand(seed);

    std::vector<int>test = unsorted;

    for (int k = 0; k < test.size(); k++) {
        int r = k + rand() % (test.size() - k);

        int t = test[k];
        test[k] = test[r];
        test[r] = t;
    }

    for (int i = 0; i < test.size() - 1; i++) {
        if (test[i] > test[i + 1]) {
            seed++;
            goto start;
        }
    }

    std::cout << "perfect seed: " << seed << std::endl;
    return test;
}