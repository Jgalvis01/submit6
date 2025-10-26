/**
 * Parallel Maximum Algorithm using OpenMP
 * 
 * This implementation finds the maximum value in an array using
 * parallel reduction with OpenMP.
 * 
 * Compilation: g++ -fopenmp parallel_maximum.cpp -o parallel_maximum
 * Execution: ./parallel_maximum
 */

#include <iostream>
#include <vector>
#include <omp.h>
#include <algorithm>
#include <climits>
#include <cstdlib>
#include <ctime>

using namespace std;

/**
 * Method 1: Parallel Maximum using OpenMP reduction clause
 * This is the simplest and most efficient approach
 * 
 * Time Complexity: O(N) work, O(log N) span
 * Synchronization: log2(N) implicit barriers
 */
int parallel_max_reduction(const vector<int>& arr, int n) {
    int max_val = INT_MIN;
    
    #pragma omp parallel for reduction(max:max_val)
    for (int i = 0; i < n; i++) {
        if (arr[i] > max_val) {
            max_val = arr[i];
        }
    }
    
    return max_val;
}

/**
 * Method 2: Parallel Maximum using manual tree reduction
 * This implementation shows the explicit tree-based reduction
 * similar to the abstract pseudocode
 * 
 * Time Complexity: O(N) work, O(log N) span
 * Synchronization: log2(N) explicit barriers
 */
int parallel_max_tree_reduction(vector<int>& arr, int n) {
    vector<int> temp(arr);  // Working array
    
    int stride = 1;
    
    // Tree reduction phase
    while (stride < n) {
        #pragma omp parallel for
        for (int i = 0; i < n; i += 2 * stride) {
            if (i + stride < n) {
                temp[i] = max(temp[i], temp[i + stride]);
            }
        }
        // Implicit barrier at end of parallel for
        stride *= 2;
    }
    
    return temp[0];
}

/**
 * Method 3: Parallel Maximum using parallel sections
 * Divides array into chunks and finds max in each chunk
 * 
 * Time Complexity: O(N) work, O(log N) span with P processors
 */
int parallel_max_sections(const vector<int>& arr, int n) {
    int num_threads = omp_get_max_threads();
    vector<int> partial_max(num_threads, INT_MIN);
    
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        int chunk_size = (n + num_threads - 1) / num_threads;
        int start = tid * chunk_size;
        int end = min(start + chunk_size, n);
        
        // Each thread finds max in its chunk
        for (int i = start; i < end; i++) {
            if (arr[i] > partial_max[tid]) {
                partial_max[tid] = arr[i];
            }
        }
    }
    
    // Final reduction in sequential
    int max_val = INT_MIN;
    for (int i = 0; i < num_threads; i++) {
        max_val = max(max_val, partial_max[i]);
    }
    
    return max_val;
}

/**
 * Method 4: Parallel Maximum with explicit barrier synchronization
 * Shows the synchronization steps clearly
 */
int parallel_max_explicit_barriers(vector<int>& arr, int n) {
    vector<int> temp(arr);
    
    // Calculate number of levels (synchronization steps)
    int levels = 0;
    int temp_n = n;
    while (temp_n > 1) {
        temp_n = (temp_n + 1) / 2;
        levels++;
    }
    
    cout << "Number of synchronization steps: " << levels << endl;
    
    // Tree reduction with explicit barriers
    for (int level = 0; level < levels; level++) {
        int stride = 1 << level;  // 2^level
        int step = stride * 2;
        
        #pragma omp parallel for
        for (int i = 0; i < n; i += step) {
            if (i + stride < n) {
                temp[i] = max(temp[i], temp[i + stride]);
            }
        }
        
        #pragma omp barrier
        
        cout << "After level " << level << " (stride=" << stride << "): ";
        for (int i = 0; i < min(n, 16); i++) {
            cout << temp[i] << " ";
        }
        cout << endl;
    }
    
    return temp[0];
}

/**
 * Sequential maximum for comparison
 */
int sequential_max(const vector<int>& arr, int n) {
    int max_val = INT_MIN;
    for (int i = 0; i < n; i++) {
        if (arr[i] > max_val) {
            max_val = arr[i];
        }
    }
    return max_val;
}

/**
 * Function to print array
 */
void print_array(const vector<int>& arr, const string& name) {
    cout << name << ": [";
    for (size_t i = 0; i < arr.size(); i++) {
        cout << arr[i];
        if (i < arr.size() - 1) cout << ", ";
    }
    cout << "]" << endl;
}

int main() {
    // Seed for random number generation
    srand(time(NULL));
    
    cout << "==================================================" << endl;
    cout << "    PARALLEL MAXIMUM ALGORITHM (OpenMP)" << endl;
    cout << "==================================================" << endl;
    cout << endl;
    
    // Get array size from user
    int n;
    cout << "Ingrese el tamaño del arreglo: ";
    cin >> n;
    
    if (n <= 0) {
        cout << "Error: El tamaño debe ser mayor que 0" << endl;
        return 1;
    }
    
    // Generate random array
    vector<int> arr(n);
    cout << "\nGenerando arreglo aleatorio de " << n << " elementos..." << endl;
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % 1000;  // Random values between 0 and 999
    }
    
    cout << endl;
    
    // Print array only if it's small enough
    if (n <= 20) {
        print_array(arr, "Input Array A");
    } else {
        cout << "Input Array A (primeros 20 elementos): [";
        for (int i = 0; i < 20; i++) {
            cout << arr[i];
            if (i < 19) cout << ", ";
        }
        cout << ", ...]" << endl;
    }
    cout << endl;
    
    // Set number of threads
    int num_threads = 4;
    omp_set_num_threads(num_threads);
    cout << "Number of OpenMP threads: " << num_threads << endl;
    cout << endl;
    
    // Method 1: OpenMP reduction
    cout << "--- Method 1: OpenMP Reduction Clause ---" << endl;
    double start = omp_get_wtime();
    int max1 = parallel_max_reduction(arr, n);
    double end = omp_get_wtime();
    cout << "Maximum value: " << max1 << endl;
    cout << "Time: " << (end - start) * 1000 << " ms" << endl;
    cout << endl;
    
    // Method 2: Tree reduction
    cout << "--- Method 2: Manual Tree Reduction ---" << endl;
    vector<int> arr2 = arr;
    start = omp_get_wtime();
    int max2 = parallel_max_tree_reduction(arr2, n);
    end = omp_get_wtime();
    cout << "Maximum value: " << max2 << endl;
    cout << "Time: " << (end - start) * 1000 << " ms" << endl;
    cout << endl;
    
    // Method 3: Parallel sections
    cout << "--- Method 3: Parallel Sections ---" << endl;
    start = omp_get_wtime();
    int max3 = parallel_max_sections(arr, n);
    end = omp_get_wtime();
    cout << "Maximum value: " << max3 << endl;
    cout << "Time: " << (end - start) * 1000 << " ms" << endl;
    cout << endl;
    
    // Method 4: Explicit barriers (shows synchronization steps)
    cout << "--- Method 4: Explicit Barriers (Debug Mode) ---" << endl;
    vector<int> arr4 = arr;
    start = omp_get_wtime();
    int max4 = parallel_max_explicit_barriers(arr4, n);
    end = omp_get_wtime();
    cout << "Maximum value: " << max4 << endl;
    cout << "Time: " << (end - start) * 1000 << " ms" << endl;
    cout << endl;
    
    // Sequential for comparison
    cout << "--- Sequential Maximum (for comparison) ---" << endl;
    start = omp_get_wtime();
    int max_seq = sequential_max(arr, n);
    end = omp_get_wtime();
    cout << "Maximum value: " << max_seq << endl;
    cout << "Time: " << (end - start) * 1000 << " ms" << endl;
    cout << endl;
    
    // Verification
    cout << "==================================================" << endl;
    cout << "VERIFICATION" << endl;
    cout << "==================================================" << endl;
    cout << "All methods found maximum: " << max1 << endl;
    bool all_correct = (max1 == max2 && max2 == max3 && max3 == max4 && max4 == max_seq);
    cout << "Status: " << (all_correct ? "PASSED ✓" : "FAILED ✗") << endl;
    
    return 0;
}
