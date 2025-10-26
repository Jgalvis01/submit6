# Algoritmos Paralelos - Maximum y Prefix Sum

## Descripción

Este proyecto implementa dos algoritmos paralelos fundamentales usando OpenMP en C++:

1. **Parallel Maximum**: Encuentra el valor máximo en un arreglo
2. **Prefix Sum (SCAN)**: Calcula las sumas acumuladas de un arreglo

## Archivos del Proyecto

```
submit6/
├── parallel_maximum.cpp      # Implementación de Parallel Maximum
├── parallel_maximum.md       # Documentación teórica y diseño
├── prefix_sum_scan.cpp       # Implementación de Prefix Sum  
├── prefix_sum_scan.md        # Documentación teórica y diseño
└── README.md                 # Este archivo
```

## Compilación

### Windows (PowerShell con MinGW):
```powershell
g++ -fopenmp parallel_maximum.cpp -o parallel_maximum.exe
g++ -fopenmp prefix_sum_scan.cpp -o prefix_sum_scan.exe
```

### Linux/Mac:
```bash
g++ -fopenmp parallel_maximum.cpp -o parallel_maximum
g++ -fopenmp prefix_sum_scan.cpp -o prefix_sum_scan
```

## Ejecución

### Parallel Maximum:
```bash
./parallel_maximum.exe

# El programa solicitará:
Ingrese el tamaño del arreglo: 8

# Genera automáticamente valores aleatorios (0-999)
# Salida: Valor máximo encontrado
```

### Prefix Sum:
```bash
./prefix_sum_scan.exe

# El programa solicitará:
Ingrese el tamaño del arreglo: 8

# Genera automáticamente valores aleatorios (1-100)
# Salida: Arreglo con sumas acumuladas
```

## Características

### Parallel Maximum
- **4 métodos implementados**: OpenMP Reduction, Tree Reduction, Parallel Sections, Explicit Barriers
- **Entrada**: Usuario ingresa tamaño N → genera arreglo aleatorio (0-999)
- **Sincronización**: ⌈log₂(N)⌉ pasos
- **Complejidad**: O(N) trabajo, O(log N) span

### Prefix Sum (SCAN)
- **3 métodos implementados**: Blelloch Scan (Upsweep+Downsweep), Divide & Conquer, Sequential
- **Entrada**: Usuario ingresa tamaño N → genera arreglo aleatorio (1-100)
- **Sincronización**: 2·⌈log₂(N)⌉ pasos (Upsweep + Downsweep)
- **Complejidad**: O(N) trabajo, O(log N) span

## Pasos de Sincronización

### Parallel Maximum
| N | Pasos de Sincronización |
|---|------------------------|
| 8 | 3 |
| 16 | 4 |
| 1000 | 10 |

**Fórmula**: ⌈log₂(N)⌉

### Prefix Sum
| N | Pasos de Sincronización |
|---|------------------------|
| 8 | 6 (3 upsweep + 3 downsweep) |
| 16 | 8 (4 upsweep + 4 downsweep) |
| 1000 | 20 (10 upsweep + 10 downsweep) |

**Fórmula**: 2·⌈log₂(N)⌉

## Documentación

Para información detallada sobre el diseño, pseudocódigo y análisis de cada algoritmo, consultar:
- **`parallel_maximum.md`** - Teoría completa del algoritmo de máximo paralelo
- **`prefix_sum_scan.md`** - Teoría completa del algoritmo de prefix sum

Cada archivo .md incluye:
1. Diseño del algoritmo paralelo
2. Pseudocódigo abstracto
3. Análisis de pasos de sincronización para N elementos
4. Ejemplos detallados paso a paso

## Configuración Opcional

Configurar número de threads de OpenMP:

```bash
# Windows PowerShell:
$env:OMP_NUM_THREADS=4

# Linux/Mac:
export OMP_NUM_THREADS=4
```

## Requisitos

- Compilador C++ con soporte OpenMP (g++, clang++, MSVC)
- OpenMP 3.0 o superior
