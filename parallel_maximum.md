# Parallel Maximum Algorithm

## 1. Algorithm Design

The parallel maximum algorithm uses a **divide-and-conquer** approach with **parallel reduction**. The idea is to divide the array into smaller parts, find the maximum in each part concurrently, and then combine these partial results.

### Key Concepts:
- **Parallel Reduction**: A tree-based approach where pairs of elements are compared in parallel
- **Logarithmic Depth**: The algorithm requires O(log N) steps instead of O(N) sequential steps
- **Work Efficiency**: Total work is O(N), same as sequential, but with better span
- **Input**: User enters array size N, program generates random values (0-999)
- **Output**: Maximum value found using multiple parallel methods

### Visual Representation (Example with N=8):
```
Input: Usuario ingresa N = 8
Generated Array: [456, 789, 123, 890, 234, 567, 345, 678]

Step 1: Compare pairs (4 comparisons in parallel)
        456 vs 789 → 789
        123 vs 890 → 890
        234 vs 567 → 567
        345 vs 678 → 678

Step 2: Compare results (2 comparisons in parallel)
        789 vs 890 → 890
        567 vs 678 → 678

Step 3: Final comparison (1 comparison)
        890 vs 678 → 890

Result: Maximum = 890
Number of Synchronization Steps: 3 (⌈log₂(8)⌉)
```

## 2. Abstract Pseudocode (Parallel Reduction)

```pseudocode
PARALLEL_MAXIMUM(A, n)
    Input: Array A of size n
    Output: Maximum value in A
    
    // Initialize: Each element is a local maximum
    for i = 0 to n-1 in parallel do
        temp[i] = A[i]
    end for
    
    // Reduction phase
    stride = 1
    while stride < n do
        for i = 0 to n-1 step 2*stride in parallel do
            if i + stride < n then
                temp[i] = max(temp[i], temp[i + stride])
            end if
        end for
        synchronize()  // Barrier synchronization
        stride = stride * 2
    end while
    
    return temp[0]
END
```

### Alternative: Tree-Based Reduction

```pseudocode
PARALLEL_MAX_TREE(A, n)
    Input: Array A of size n (assume n is power of 2)
    Output: Maximum value in A
    
    B = A  // Working array
    
    for d = 0 to log₂(n) - 1 do
        // At depth d, we have n/2^d active elements
        active = n / 2^(d+1)
        
        for i = 0 to active-1 in parallel do
            B[i] = max(B[2*i], B[2*i + 1])
        end for
        
        synchronize()  // Barrier after each level
    end for
    
    return B[0]
END
```

## 3. Synchronization Steps Required

For an array of **N elements**:

### Number of Synchronization Steps:
**⌈log₂(N)⌉** synchronization barriers

### Detailed Analysis:

- **N = 8 elements**: ⌈log₂(8)⌉ = 3 steps
  - Step 1: 8 → 4 partial maxima
  - Step 2: 4 → 2 partial maxima  
  - Step 3: 2 → 1 final maximum

- **N = 16 elements**: ⌈log₂(16)⌉ = 4 steps

- **General case**: ⌈log₂(N)⌉ steps

### Why log₂(N)?
Each synchronization step reduces the problem size by half, forming a binary tree of depth log₂(N).

## Complexity Analysis

| Metric | Value | Description |
|--------|-------|-------------|
| **Work** | O(N) | Total number of comparisons |
| **Span** | O(log N) | Critical path length |
| **Parallelism** | O(N/log N) | Average available parallelism |
| **Processors** | O(N/2) max | At first step, N/2 comparisons |
| **Speedup** | O(N/log N) | Theoretical speedup vs sequential |

## Implementation Considerations

1. **Padding**: If N is not a power of 2, pad with -∞ or handle boundary cases
2. **Memory**: Requires O(N) space for temporary array
3. **Load Balancing**: Tree structure naturally balances work
4. **Communication**: Minimize by using shared memory when possible
5. **Random Generation**: Values generated between 0-999 for testing
6. **User Input**: Program prompts for array size N

## C++ Implementation with OpenMP

The implementation (`parallel_maximum.cpp`) includes **4 methods**:

### Method 1: OpenMP Reduction Clause
- Most simple and efficient
- Uses `#pragma omp parallel for reduction(max:max_val)`
- Implicit synchronization handled by OpenMP
- Best for production use

### Method 2: Manual Tree Reduction
- Explicit tree-based reduction
- Shows the algorithm structure clearly
- Uses explicit stride doubling
- Educational purpose - shows exact algorithm steps

### Method 3: Parallel Sections (Chunk-based)
- Divides array into chunks per thread
- Each thread finds local maximum
- Final reduction in sequential
- Good for understanding block decomposition

### Method 4: Explicit Barriers (Debug Mode)
- Shows all synchronization steps explicitly
- Prints intermediate array states at each level
- Displays number of synchronization barriers
- Best for understanding the algorithm flow

## Example Execution

### Input:
```
Ingrese el tamaño del arreglo: 8
```

### Generated Array (random):
```
[456, 789, 123, 890, 234, 567, 345, 678]
```

### Output with Method 4 (Debug):
```
Number of synchronization steps: 3
After level 0 (stride=1): 456 789 123 890 234 567 345 678
After level 1 (stride=2): 789 789 890 890 567 567 678 678
After level 2 (stride=4): 890 789 890 890 678 567 678 678
Maximum value: 890
```

## Example Output for Different Array Sizes

```
Given Input N and Synchronization Steps:

N = 8 elements:  ⌈log₂(8)⌉  = 3 steps
N = 16 elements: ⌈log₂(16)⌉ = 4 steps
N = 32 elements: ⌈log₂(32)⌉ = 5 steps
N = 1000 elements: ⌈log₂(1000)⌉ = 10 steps
N = 10000 elements: ⌈log₂(10000)⌉ = 14 steps
```
