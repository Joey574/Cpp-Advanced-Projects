#include <vector>
#include <iostream>
#include <string>
#include <chrono>
#include <Windows.h>
#include <numeric>

#define RED_TEXT 4
#define GREEN_TEXT 10
#define WHITE_TEXT 7
#define BLUE_TEXT 1
#define YELLOW_TEXT 6
#define PURPLE_TEXT 13

void run_test(std::vector<int>(*sorter)(std::vector<int>), std::string name);
void simple_test(std::vector<int>(*sorter)(std::vector<int>));
void shuffle(std::vector<int>& a);

std::vector<int> selecition_sort(std::vector<int> unsorted);
std::vector<int> insertion_sort(std::vector<int> unsorted);
std::vector<int> bubble_sort(std::vector<int> unsorted);
std::vector<int> merge_sort(std::vector<int> unsorted);
std::vector<int> three_way_merge_sort(std::vector<int> unsorted);
std::vector<int> comb_sort(std::vector<int> unsorted);
std::vector<int> tim_sort(std::vector<int> unsorted);
std::vector<int> countintg_sort(std::vector<int> unsorted);
std::vector<int> shell_sort(std::vector<int> unsorted);

std::vector<int> bogo_sort(std::vector<int> unsorted);
std::vector<int> bogo_seed_find(std::vector<int> unsorted);

int main()
{
    //run_test(selecition_sort, "selection sort");
    //run_test(insertion_sort, "insertion sort");
    //run_test(bubble_sort, "bubble sort");
    //run_test(merge_sort, "merge sort");
    run_test(three_way_merge_sort, "three_way_merge_sort");
    //run_test(countintg_sort, "countintg sort");
    //run_test(comb_sort, "comb sort");
    //run_test(tim_sort, "tim sort"); // -> needs to be made
    run_test(shell_sort, "shell sort");

    //run_test(bogo_sort, "bogo sort");
    //run_test(bogo_seed_find, "seed find: ");
}

void shuffle(std::vector<int>& a) {
    for (int k = 0; k < a.size(); k++) {
        int r = k + rand() % (a.size() - k);

        int t = a[k];
        a[k] = a[r];
        a[r] = t;
    }
}
void run_test(std::vector<int>(*sorter)(std::vector<int>), std::string name) {

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, YELLOW_TEXT); std::cout << name << ":\n";

    const int max_size = 8192;
    const int multiplier = 2;
    int runs = 4096;

    srand(0);

    for (int size = 1024; size <= max_size; size *= multiplier, runs /= multiplier) {
        std::vector<int> to_sort(size);

        runs = runs < 8 ? 8 : runs;

        for (int i = 0; i < to_sort.size(); i++) {
            //to_sort[i] = i;
            to_sort[i] = rand() % 100000;
        }

        shuffle(to_sort);

        std::vector<double> best(runs);

        // warmup run
        sorter(to_sort);

        for (int r = 0; r < runs; r++) {
            auto start = std::chrono::high_resolution_clock::now();
            std::vector<int> sorted = sorter(to_sort);
            best[r] = (std::chrono::high_resolution_clock::now() - start).count() / 1000000.00;

            for (int i = 0; i < sorted.size() - 1; i++) {
                if (sorted[i] > sorted[i + 1]) {
                    std::cout << name << " failed\n";
                    break;
                }
            }
        }
        
        double min = *std::min_element(&best[0], &best[runs]);
        double max = *std::max_element(&best[0], &best[runs]);
        double sum = std::accumulate(&best[0], &best[runs], 0.0);

        SetConsoleTextAttribute(hConsole, WHITE_TEXT); std::cout << "\t" << size << ": ";
        SetConsoleTextAttribute(hConsole, GREEN_TEXT); std::cout << min << "ms";
        SetConsoleTextAttribute(hConsole, WHITE_TEXT); std::cout << " - ";
        SetConsoleTextAttribute(hConsole, RED_TEXT); std::cout << max << "ms";
        SetConsoleTextAttribute(hConsole, WHITE_TEXT); std::cout << " :: ";
        SetConsoleTextAttribute(hConsole, BLUE_TEXT); std::cout << (sum / (double)runs) << "ms";
        SetConsoleTextAttribute(hConsole, WHITE_TEXT); std::cout << " taken over ";
        SetConsoleTextAttribute(hConsole, YELLOW_TEXT); std::cout << runs;
        SetConsoleTextAttribute(hConsole, WHITE_TEXT); std::cout << " runs\n";

        SetConsoleTextAttribute(hConsole, WHITE_TEXT);
    }
}
void simple_test(std::vector<int>(*sorter)(std::vector<int>)) {
    std::vector<int> to_sort(100);

    for (int i = 0; i < to_sort.size(); i++) {
        to_sort[i] = i;
    }

    shuffle(to_sort);

    to_sort = sorter(to_sort);

    for (int i = 0; i < to_sort.size(); i++) {
        std::cout << to_sort[i] << " ";
    } std::cout << "\n";
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
std::vector<int> three_way_merge_sort(std::vector<int> unsorted) {
    if (unsorted.size() < 2) {
        return unsorted;
    }

    std::vector<int> a = three_way_merge_sort(std::vector<int>(unsorted.begin(), unsorted.begin() + (unsorted.size() / 3)));
    std::vector<int> b = three_way_merge_sort(std::vector<int>(unsorted.begin() + (unsorted.size() / 3), unsorted.begin() + ( 2 * unsorted.size() / 3)));
    std::vector<int> c = three_way_merge_sort(std::vector<int>(unsorted.begin() + (2 * unsorted.size() / 3), unsorted.end()));

    int a_idx = 0;
    int b_idx = 0;
    int c_idx = 0;

    for (int i = 0; i < unsorted.size(); i++) {

        if (a_idx > a.size()) {

        }
 
        // set element to min of a, b, and c and adjust indexes
        if (a[a_idx] < b[b_idx] && a[a_idx] < c[c_idx]) {
            unsorted[i] = a[a_idx];
            a_idx++;
        } else if (b[b_idx] < a[a_idx] && b[b_idx] < c[c_idx]) {
            unsorted[i] = b[b_idx];
            b_idx++;
        } else {
            unsorted[i] = c[c_idx];
            c_idx++;
        }
    }

    return unsorted;
}
std::vector<int> comb_sort(std::vector<int> unsorted) {

    int gap = unsorted.size();
    bool swap = true;

    while (gap > 1 || swap) {

        gap = (gap * 10 / 13) < 1 ? 1 : (gap * 10 / 13);
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

    for (int i = 0; i <= unsorted.size(); i += run_size, run_size *= 2) {
        auto t = insertion_sort({ &unsorted[i], &unsorted[i + run_size]});

        // copy t into unsorted
        std::copy(&t[0], &t[run_size], &unsorted[i]);
    }

    /*
    
    Let痴 consider the following array as an example: arr[] = {4, 2, 8, 6, 1, 5, 9, 3, 7}.

Step 1: Define the size of the run

Minimum run size: 32 (we値l ignore this step since our array is small)
Step 2: Divide the array into runs

In this step, we値l use insertion sort to sort the small subsequences (runs) within the array.
The initial array: [4, 2, 8, 6, 1, 5, 9, 3, 7]
No initial runs are present, so we値l create runs using insertion sort.
Sorted runs: [2, 4], [6, 8], [1, 5, 9], [3, 7]
Updated array: [2, 4, 6, 8, 1, 5, 9, 3, 7]
Step 3: Merge the runs

In this step, we値l merge the sorted runs using a modified merge sort algorithm.
Merge the runs until the entire array is sorted.
Merged runs: [2, 4, 6, 8], [1, 3, 5, 7, 9]
Updated array: [2, 4, 6, 8, 1, 3, 5, 7, 9]
Step 4: Adjust the run size

After each merge operation, we double the size of the run until it exceeds the length of the array.
The run size doubles: 32, 64, 128 (we値l ignore this step since our array is small)
Step 5: Continue merging

Repeat the merging process until the entire array is sorted.
Final merged run: [1, 2, 3, 4, 5, 6, 7, 8, 9]
The final sorted array is [1, 2, 3, 4, 5, 6, 7, 8, 9].


    
    */

    return unsorted;
}
std::vector<int> countintg_sort(std::vector<int> unsorted) {
    int m = *std::max_element(unsorted.begin(), unsorted.end());

    std::vector<u_int> expand(m + 1, 0);

    // set indexes
    for (int i = 0; i < unsorted.size(); i++) {
        expand[unsorted[i]]++;
    }

    size_t idx = 0;
    for (size_t i = 0; i < expand.size(); i++) {
        for (size_t j = 0; j < expand[i]; j++) {
            unsorted[idx] = i;
            idx++;
        }
    }

    return unsorted;
}
std::vector<int> shell_sort(std::vector<int> unsorted) {
    for (int gap = unsorted.size() / 2; gap > 0; gap /= 2) {

        for (int i = gap; i < unsorted.size(); i++)
        {
            int t = unsorted[i];
            int j;

            for (j = i; j >= gap && unsorted[j - gap] > t; j -= gap) {
                unsorted[j] = unsorted[j - gap];
            }

            unsorted[j] = t;
        }
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