# Parallel Prefix Sum (SCAN) Algorithm

## Understanding Prefix Sum

**Prefix Sum** (also called **cumulative sum** or **SCAN**): Given an array A, compute an array P where each element P[i] is the sum of all elements A[0] through A[i].

### How the Program Works:
1. **Input**: User enters array size N
2. **Generation**: Program creates random array with values 1-100
3. **Processing**: Computes prefix sum using parallel algorithms
4. **Output**: Array P with cumulative sums

### Example:
```
Input: N = 8
Generated Array A:  [12, 34, 21, 45, 23, 18, 36, 29]
Output Array P:     [12, 46, 67, 112, 135, 153, 189, 218]

Explanation:
P[0] = A[0] = 12
P[1] = A[0] + A[1] = 12 + 34 = 46
P[2] = A[0] + A[1] + A[2] = 12 + 34 + 21 = 67
P[3] = 12 + 34 + 21 + 45 = 112
P[4] = 112 + 23 = 135
P[5] = 135 + 18 = 153
P[6] = 153 + 36 = 189
P[7] = 189 + 29 = 218
```

## 1. Algorithm Design

The **Blelloch Scan** algorithm uses a two-phase approach:
1. **Upsweep (Reduce Phase)**: Build a reduction tree (similar to parallel max)
2. **Downsweep Phase**: Propagate partial sums down the tree

This is a **work-efficient** algorithm with O(N) work instead of O(N log N).

### Visual Representation (Example with generated data):

```
Input: N = 8
Generated: [12, 34, 21, 45, 23, 18, 36, 29]

UPSWEEP PHASE (bottom-up reduction):
═══════════════════════════════════════════════════════════════
Level 0: 12   34   21   45   23   18   36   29
          └─+─┘    └─+─┘    └─+─┘    └─+─┘
Level 1: 12   46   21   66   23   41   36   65    (step=1, Sync 1)
          └────+────┘    └────+────┘
Level 2: 12   46   21  112   23   41   36  106    (step=2, Sync 2)
          └─────────+─────────┘
Level 3: 12   46   21  112   23   41   36  218    (step=4, Sync 3)
                                          ↑
                                    (total sum)

DOWNSWEEP PHASE (top-down propagation):
═══════════════════════════════════════════════════════════════
Set root to 0:
Level 3: 12   46   21  112   23   41   36    0    (set last to 0)
          └─────────+─────────┘
Level 2: 12   46   21    0   23   41   36  112    (step=4, Sync 4)
          └────+────┘    └────+────┘
Level 1: 12    0   21   46   23  112   36  153    (step=2, Sync 5)
          └─+─┘    └─+─┘    └─+─┘    └─+─┘
Level 0:  0   12   46   67  112  135  153  189    (step=1, Sync 6)

Add original values: [12, 46, 67, 112, 135, 153, 189, 218] ✓

Total Synchronization Steps: 6 (3 upsweep + 3 downsweep)
```

## 2. Abstract Pseudocode (Two-Phase Approach)

### Complete Blelloch Scan Algorithm:

```pseudocode
PARALLEL_PREFIX_SUM(A, n)
    Input: Array A of size n (assume n is power of 2)
    Output: Array P containing prefix sums
    
    // Copy input to working array
    P = copy(A)
    
    // ═══════════════════════════════════════════════════
    // PHASE 1: UPSWEEP (Reduce) - Build reduction tree
    // ═══════════════════════════════════════════════════
    for d = 0 to log₂(n) - 1 do
        stride = 2^(d+1)
        
        for i = 0 to n-1 step stride in parallel do
            // Add left child to right child
            P[i + stride - 1] = P[i + 2^d - 1] + P[i + stride - 1]
        end for
        
        synchronize()  // Barrier after each level
    end for
    
    // ═══════════════════════════════════════════════════
    // PHASE 2: DOWNSWEEP - Propagate partial sums
    // ═══════════════════════════════════════════════════
    
    // Set root to identity (0 for sum)
    P[n - 1] = 0
    
    for d = log₂(n) - 1 down to 0 do
        stride = 2^(d+1)
        
        for i = 0 to n-1 step stride in parallel do
            // Save current right child
            temp = P[i + 2^d - 1]
            
            // Right child = parent
            P[i + 2^d - 1] = P[i + stride - 1]
            
            // New right child = old right + parent
            P[i + stride - 1] = temp + P[i + stride - 1]
        end for
        
        synchronize()  // Barrier after each level
    end for
    
    return P
END
```

### Simplified Version with Clear Indexing:

```pseudocode
PARALLEL_SCAN_SIMPLIFIED(A, n)
    Input: Array A[0..n-1], n is power of 2
    Output: Prefix sum array
    
    temp = copy(A)
    
    // ───────────── UPSWEEP ─────────────
    offset = 1
    for d = n/2 down to 1 step d/2 do
        for i = 0 to d-1 in parallel do
            ai = offset * (2*i + 1) - 1
            bi = offset * (2*i + 2) - 1
            temp[bi] = temp[ai] + temp[bi]
        end for
        synchronize()
        offset = offset * 2
    end for
    
    // Clear last element
    temp[n-1] = 0
    
    // ───────────── DOWNSWEEP ─────────────
    for d = 1 to n/2 step d*2 do
        offset = offset / 2
        
        for i = 0 to d-1 in parallel do
            ai = offset * (2*i + 1) - 1
            bi = offset * (2*i + 2) - 1
            
            t = temp[ai]
            temp[ai] = temp[bi]
            temp[bi] = t + temp[bi]
        end for
        synchronize()
    end for
    
    return temp
END
```

## 3. Synchronization Steps Required

For an array of **N elements** (where N is a power of 2):

### Total Synchronization Steps:
**2 × log₂(N)** synchronization barriers

### Breakdown:
- **Upsweep Phase**: log₂(N) barriers
- **Downsweep Phase**: log₂(N) barriers

### Examples:

| N | Upsweep Steps | Downsweep Steps | Total Steps |
|---|---------------|-----------------|-------------|
| 8 | 3 | 3 | **6** |
| 16 | 4 | 4 | **8** |
| 32 | 5 | 5 | **10** |
| 1024 | 10 | 10 | **20** |
| N | log₂(N) | log₂(N) | **2·log₂(N)** |

### Detailed for N = 8:

**Upsweep (3 synchronization steps):**
1. Step 1: Combine pairs (stride=2)
2. Step 2: Combine quads (stride=4)
3. Step 3: Combine halves (stride=8)

**Downsweep (3 synchronization steps):**
1. Step 4: Split halves (stride=8)
2. Step 5: Split quads (stride=4)
3. Step 6: Split pairs (stride=2)

**Total: 6 synchronization steps**

## Complexity Analysis

| Metric | Blelloch Scan | Naive Parallel |
|--------|---------------|----------------|
| **Work** | O(N) | O(N log N) |
| **Span** | O(log N) | O(log N) |
| **Step Complexity** | O(log N) | O(log N) |
| **Synchronization** | 2·log₂(N) | log₂(N) |
| **Work Efficient** | ✓ Yes | ✗ No |

## Step-by-Step Example with Generated Data

```
Input: N = 8
Generated A: [12, 34, 21, 45, 23, 18, 36, 29]

UPSWEEP:
────────────────────────────────────────────────────────────
Initial:    [12, 34, 21, 45, 23, 18, 36, 29]

Level d=0 (stride=2, pairs):
  P[1] = P[0] + P[1] = 12 + 34 = 46
  P[3] = P[2] + P[3] = 21 + 45 = 66
  P[5] = P[4] + P[5] = 23 + 18 = 41
  P[7] = P[6] + P[7] = 36 + 29 = 65
Result:     [12, 46, 21, 66, 23, 41, 36, 65]  (Sync 1)

Level d=1 (stride=4, quads):
  P[3] = P[1] + P[3] = 46 + 66 = 112
  P[7] = P[5] + P[7] = 41 + 65 = 106
Result:     [12, 46, 21, 112, 23, 41, 36, 106]  (Sync 2)

Level d=2 (stride=8, halves):
  P[7] = P[3] + P[7] = 112 + 106 = 218
Result:     [12, 46, 21, 112, 23, 41, 36, 218]  (Sync 3)

DOWNSWEEP:
────────────────────────────────────────────────────────────
Set P[7] = 0:
            [12, 46, 21, 112, 23, 41, 36, 0]

Level d=2 (stride=8):
  temp = P[3] = 112
  P[3] = P[7] = 0
  P[7] = temp + P[7] = 112 + 0 = 112
Result:     [12, 46, 21, 0, 23, 41, 36, 112]  (Sync 4)

Level d=1 (stride=4):
  temp = P[1] = 46; P[1] = P[3] = 0; P[3] = 46 + 0 = 46
  temp = P[5] = 41; P[5] = P[7] = 112; P[7] = 41 + 112 = 153
Result:     [12, 0, 21, 46, 23, 112, 36, 153]  (Sync 5)

Level d=0 (stride=2):
  temp = P[0] = 12; P[0] = P[1] = 0; P[1] = 12 + 0 = 12
  temp = P[2] = 21; P[2] = P[3] = 46; P[3] = 21 + 46 = 67
  temp = P[4] = 23; P[4] = P[5] = 112; P[5] = 23 + 112 = 135
  temp = P[6] = 36; P[6] = P[7] = 153; P[7] = 36 + 153 = 189
Result:     [0, 12, 46, 67, 112, 135, 153, 189]  (Sync 6)

FINAL (Exclusive Scan): [0, 12, 46, 67, 112, 135, 153, 189]
For Inclusive Scan, add original: [12, 46, 67, 112, 135, 153, 189, 218] ✓

Total Synchronization Steps: 6
Total Sum: 218
```

## Notes

- **Exclusive Scan**: P[i] = sum of A[0..i-1] (P[0] = 0)
- **Inclusive Scan**: P[i] = sum of A[0..i] (what the program outputs)
- The pseudocode above computes **exclusive scan**; add original values for inclusive
- **Work-efficient**: Uses only O(N) operations total (not O(N log N))
- Requires N to be a power of 2 (pad with zeros if needed - automatic in implementation)
- **Random Values**: Program generates values between 1-100
- **User Input**: Program prompts for array size N

## C++ Implementation with OpenMP

The implementation (`prefix_sum_scan.cpp`) includes **3 methods**:

### Method 1: Blelloch Scan (Two-Phase)
- Complete implementation of Upsweep + Downsweep
- Shows all intermediate steps
- Displays synchronization barriers explicitly
- Pads array to next power of 2 automatically
- Prints state at each level for debugging
- Best for understanding the algorithm

### Method 2: Divide and Conquer (Block-based)
- Divides array into blocks per thread
- Computes local prefix sums in parallel
- Sequential scan of block sums
- Adds block offsets in parallel
- More practical for production use

### Method 3: Sequential (Reference)
- Standard sequential scan
- Used for verification
- Baseline for performance comparison

## Example Execution

### Input:
```
Ingrese el tamaño del arreglo: 8
```

### Generated Array (random):
```
[12, 34, 21, 45, 23, 18, 36, 29]
```

### Output Summary:
```
Number of elements (padded): 8
Number of levels: 3
Synchronization steps: 6 (3 upsweep + 3 downsweep)

Result P: [12, 46, 67, 112, 135, 153, 189, 218]
Suma total: 218
Verification: PASSED ✓
```

## Synchronization Steps for Different Sizes

```
Input N and Synchronization Steps:

N = 8:     Padded to 8,    log₂(8)=3,    Steps = 2×3  = 6
N = 16:    Padded to 16,   log₂(16)=4,   Steps = 2×4  = 8
N = 100:   Padded to 128,  log₂(128)=7,  Steps = 2×7  = 14
N = 1000:  Padded to 1024, log₂(1024)=10, Steps = 2×10 = 20
N = 10000: Padded to 16384, log₂(16384)=14, Steps = 2×14 = 28
```

## Applications

1. Parallel stream compaction
2. Radix sort
3. Polynomial evaluation
4. Solving recurrences
5. Allocating memory in parallel
