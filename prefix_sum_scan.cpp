/**
 * Parallel Prefix Sum (SCAN) Algorithm using OpenMP
 * 
 * This implementation computes the prefix sum using the Blelloch algorithm
 * with two phases: Upsweep (Reduce) and Downsweep
 * 
 * Compilation: g++ -fopenmp prefix_sum_scan.cpp -o prefix_sum_scan
 * Execution: ./prefix_sum_scan
 */

#include <iostream>
#include <vector>
#include <omp.h>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <ctime>

using namespace std;

/**
 * Method 1: Blelloch Scan - Work-efficient parallel prefix sum
 * Uses two-phase approach: Upsweep (Reduce) + Downsweep
 * 
 * Time Complexity: O(N) work, O(log N) span
 * Synchronization: 2*log2(N) barriers
 * 
 * Computes INCLUSIVE scan
 */
vector<int> parallel_prefix_sum_blelloch(vector<int> arr) {
    int n = arr.size();
    
    // Ensure n is power of 2 (pad if necessary)
    int original_n = n;
    int log_n = (int)ceil(log2(n));
    n = 1 << log_n;  // Next power of 2
    
    vector<int> temp(n, 0);
    for (int i = 0; i < original_n; i++) {
        temp[i] = arr[i];
    }
    
    cout << "Number of elements (padded): " << n << endl;
    cout << "Number of levels: " << log_n << endl;
    cout << "Synchronization steps: " << (2 * log_n) << " (" << log_n << " upsweep + " << log_n << " downsweep)" << endl;
    cout << endl;
    
    // =================================================================
    // PHASE 1: UPSWEEP (Reduce) - Build reduction tree
    // =================================================================
    cout << "--- UPSWEEP PHASE ---" << endl;
    cout << "Initial: ";
    for (int i = 0; i < min(n, 16); i++) cout << temp[i] << " ";
    cout << endl;
    
    for (int d = 0; d < log_n; d++) {
        int stride = 1 << (d + 1);  // 2^(d+1)
        int offset = (1 << d) - 1;  // 2^d - 1
        
        #pragma omp parallel for
        for (int i = 0; i < n; i += stride) {
            temp[i + stride - 1] += temp[i + offset];
        }
        
        #pragma omp barrier
        
        cout << "Level " << d << " (stride=" << stride << "): ";
        for (int i = 0; i < min(n, 16); i++) cout << temp[i] << " ";
        cout << endl;
    }
    
    cout << endl;
    
    // =================================================================
    // PHASE 2: DOWNSWEEP - Propagate partial sums down the tree
    // =================================================================
    cout << "--- DOWNSWEEP PHASE ---" << endl;
    
    // Set root to 0 (for exclusive scan)
    // For inclusive scan, we'll adjust at the end
    int total_sum = temp[n - 1];
    temp[n - 1] = 0;
    
    cout << "Set root to 0: ";
    for (int i = 0; i < min(n, 16); i++) cout << temp[i] << " ";
    cout << endl;
    
    for (int d = log_n - 1; d >= 0; d--) {
        int stride = 1 << (d + 1);  // 2^(d+1)
        int offset = (1 << d) - 1;  // 2^d - 1
        
        #pragma omp parallel for
        for (int i = 0; i < n; i += stride) {
            int t = temp[i + offset];
            temp[i + offset] = temp[i + stride - 1];
            temp[i + stride - 1] += t;
        }
        
        #pragma omp barrier
        
        cout << "Level " << (log_n - 1 - d) << " (stride=" << stride << "): ";
        for (int i = 0; i < min(n, 16); i++) cout << temp[i] << " ";
        cout << endl;
    }
    
    cout << endl;
    cout << "Result (Exclusive): ";
    for (int i = 0; i < min(n, 16); i++) cout << temp[i] << " ";
    cout << endl;
    
    // Convert exclusive scan to inclusive scan
    // Inclusive[i] = Exclusive[i] + Original[i]
    vector<int> result(original_n);
    for (int i = 0; i < original_n; i++) {
        result[i] = temp[i] + arr[i];
    }
    
    return result;
}

/**
 * Method 2: Simple Parallel Prefix Sum using OpenMP scan directive
 * (Available in OpenMP 5.0+)
 * 
 * This is simpler but may not be available in all OpenMP implementations
 */
vector<int> parallel_prefix_sum_omp_scan(const vector<int>& arr) {
    int n = arr.size();
    vector<int> result(n);
    
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            result[i] = arr[i];
        }
        
        // Note: OpenMP scan directive might not be available in all versions
        // This is a simplified version
        #pragma omp single
        {
            result[0] = arr[0];
            for (int i = 1; i < n; i++) {
                result[i] = result[i-1] + arr[i];
            }
        }
    }
    
    return result;
}

/**
 * Method 3: Parallel Prefix Sum using divide and conquer
 * Good for understanding the parallel decomposition
 */
vector<int> parallel_prefix_sum_recursive(const vector<int>& arr) {
    int n = arr.size();
    vector<int> result(n);
    
    if (n == 0) return result;
    
    int num_threads = omp_get_max_threads();
    vector<int> block_sums(num_threads, 0);
    
    // Phase 1: Compute prefix sum in each block
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        int chunk_size = (n + num_threads - 1) / num_threads;
        int start = tid * chunk_size;
        int end = min(start + chunk_size, n);
        
        if (start < end) {
            result[start] = arr[start];
            int local_sum = arr[start];
            
            for (int i = start + 1; i < end; i++) {
                local_sum += arr[i];
                result[i] = local_sum;
            }
            
            block_sums[tid] = local_sum;
        }
    }
    
    // Phase 2: Compute prefix sum of block sums (sequential for simplicity)
    vector<int> block_prefix(num_threads);
    block_prefix[0] = 0;
    for (int i = 1; i < num_threads; i++) {
        block_prefix[i] = block_prefix[i-1] + block_sums[i-1];
    }
    
    // Phase 3: Add block prefix to each element
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        int chunk_size = (n + num_threads - 1) / num_threads;
        int start = tid * chunk_size;
        int end = min(start + chunk_size, n);
        
        for (int i = start; i < end; i++) {
            result[i] += block_prefix[tid];
        }
    }
    
    return result;
}

/**
 * Sequential prefix sum for comparison
 */
vector<int> sequential_prefix_sum(const vector<int>& arr) {
    int n = arr.size();
    vector<int> result(n);
    
    if (n == 0) return result;
    
    result[0] = arr[0];
    for (int i = 1; i < n; i++) {
        result[i] = result[i-1] + arr[i];
    }
    
    return result;
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

/**
 * Function to verify two arrays are equal
 */
bool verify_arrays(const vector<int>& a, const vector<int>& b) {
    if (a.size() != b.size()) return false;
    
    for (size_t i = 0; i < a.size(); i++) {
        if (a[i] != b[i]) {
            cout << "Mismatch at index " << i << ": " << a[i] << " != " << b[i] << endl;
            return false;
        }
    }
    return true;
}

int main() {
    // Seed for random number generation
    srand(time(NULL));
    
    cout << "==================================================" << endl;
    cout << "    PARALLEL PREFIX SUM (SCAN) - OpenMP" << endl;
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
        arr[i] = rand() % 100 + 1;  // Random values between 1 and 100
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
    
    // Sequential reference
    cout << "==================================================" << endl;
    cout << "Sequential Prefix Sum (Reference)" << endl;
    cout << "==================================================" << endl;
    double start = omp_get_wtime();
    vector<int> result_seq = sequential_prefix_sum(arr);
    double end = omp_get_wtime();
    
    if (n <= 20) {
        print_array(result_seq, "Result P");
    } else {
        cout << "Result P (primeros 20 elementos): [";
        for (int i = 0; i < 20; i++) {
            cout << result_seq[i];
            if (i < 19) cout << ", ";
        }
        cout << ", ...]" << endl;
        cout << "Último elemento (suma total): " << result_seq[n-1] << endl;
    }
    
    cout << "Time: " << (end - start) * 1000 << " ms" << endl;
    cout << endl;
    
    // Method 1: Blelloch Scan (most detailed)
    cout << "==================================================" << endl;
    cout << "Method 1: Blelloch Scan (Two-Phase)" << endl;
    cout << "==================================================" << endl;
    start = omp_get_wtime();
    vector<int> result1 = parallel_prefix_sum_blelloch(arr);
    end = omp_get_wtime();
    
    if (n <= 20) {
        print_array(result1, "Result P");
    } else {
        cout << "Result P (primeros 20 elementos): [";
        for (int i = 0; i < 20; i++) {
            cout << result1[i];
            if (i < 19) cout << ", ";
        }
        cout << ", ...]" << endl;
        cout << "Último elemento (suma total): " << result1[n-1] << endl;
    }
    
    cout << "Time: " << (end - start) * 1000 << " ms" << endl;
    cout << "Verification: " << (verify_arrays(result1, result_seq) ? "PASSED ✓" : "FAILED ✗") << endl;
    cout << endl;
    
    // Method 3: Divide and Conquer
    cout << "==================================================" << endl;
    cout << "Method 2: Divide and Conquer (Block-based)" << endl;
    cout << "==================================================" << endl;
    start = omp_get_wtime();
    vector<int> result3 = parallel_prefix_sum_recursive(arr);
    end = omp_get_wtime();
    
    if (n <= 20) {
        print_array(result3, "Result P");
    } else {
        cout << "Result P (primeros 20 elementos): [";
        for (int i = 0; i < 20; i++) {
            cout << result3[i];
            if (i < 19) cout << ", ";
        }
        cout << ", ...]" << endl;
        cout << "Último elemento (suma total): " << result3[n-1] << endl;
    }
    
    cout << "Time: " << (end - start) * 1000 << " ms" << endl;
    cout << "Verification: " << (verify_arrays(result3, result_seq) ? "PASSED ✓" : "FAILED ✗") << endl;
    cout << endl;
    
    // Final summary
    cout << "==================================================" << endl;
    cout << "RESUMEN" << endl;
    cout << "==================================================" << endl;
    cout << "Tamaño del arreglo: " << n << endl;
    cout << "Número de threads: " << num_threads << endl;
    cout << "Suma total: " << result_seq[n-1] << endl;
    cout << "Todos los métodos: " << (verify_arrays(result1, result_seq) && verify_arrays(result3, result_seq) ? "PASSED ✓" : "FAILED ✗") << endl;
    
    return 0;
}
