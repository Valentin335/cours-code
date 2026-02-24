# Cutting Stock

Given bars of capacity $W$ and a set of items $I$, each with size $w_i$ and demand $d_i$, find the minimum number of bars needed to satisfy all demands.

## Objective

Compare the **LP relaxations** of two formulations: a compact MILP and a Dantzig-Wolfe reformulation solved by column generation. The column generation formulation yields a tighter bound.

## Formulations

### Compact MILP

Upper bound on bars: $C = \lceil \sum_i w_i d_i \,/\, W \rceil$ (continuous relaxation bound; the trivial bound $\sum_i d_i$ is valid but much weaker).

$$
\begin{align}
\min \quad & \sum_{c=1}^{C} y_c \\
\text{s.c.} \quad & \sum_{i \in I} w_i \, x_{i,c} \leq W \, y_c & \forall c \\
& \sum_{c=1}^{C} x_{i,c} \geq d_i & \forall i \in I \\
& y_c \in \{0,1\}, \quad x_{i,c} \in \mathbb{N}
\end{align}
$$

### Column generation

A *pattern* $p$ is a feasible filling of a bar: $a_{i,p}$ copies of item $i$ with $\sum_i w_i a_{i,p} \leq W$.

**Master problem:**

$$
\begin{align}
\min \quad & \sum_{p \in \mathcal{P}} \lambda_p \\
\text{s.c.} \quad & \sum_{p \in \mathcal{P}} a_{i,p} \, \lambda_p \geq d_i & \forall i \in I \quad [\pi_i] \\
& \lambda_p \geq 0
\end{align}
$$

**Pricing subproblem** (knapsack):

$$
\max \sum_{i \in I} \pi_i^* \, a_i \quad \text{s.c.} \quad \sum_{i \in I} w_i \, a_i \leq W, \quad a_i \in \mathbb{N}
$$

A new column improves the master iff the pricing optimum exceeds 1.

**Algorithm:**

```
┌─────────────────────────────────────────┐
│  Initialization: build_master()         │
│  One trivial pattern per item type      │
└──────────────────┬──────────────────────┘
                   │
                   ▼
          ┌────────────────┐
          │  Solve PMR     │◄──────────────────┐
          │  master.optimize()                  │
          └───────┬────────┘                    │
                  │ extract π*                  │
                  ▼                             │
        ┌──────────────────┐                    │
        │  Pricing         │     red. cost < 0  │
        │  solve_pricing() ├────────────────────┘
        └────────┬─────────┘   add_column()
                 │ red. cost ≥ 0
                 ▼
        ┌──────────────────┐
        │  Optimal LP      │
        └──────────────────┘
```

## Structure

```
cutting-stock/
├── common/instance.hpp              # Shared instance parser
├── compact/src/main.cpp             # Compact MILP (LP relaxation)
├── column-generation/src/main.cpp   # Column generation (LP relaxation)
├── scripts/
│   ├── compare.sh                   # Run both on all instances
│   └── report.py                    # Update README results table
├── CMakeLists.txt
└── data/                            # Instance files
```

## Build & run

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j

# Single instance
./compact ../data/xaa 60        # 60s time limit
./colgen ../data/xaa

# All instances — generates results.csv and updates README
cd .. && bash scripts/compare.sh 120
```

## Data

Instance files go in `data/` (not tracked by git). Standard CSP benchmark instances (e.g. from [BPPLIB](https://site.unibo.it/operations-research/en/research/bpplib-a-bin-packing-problem-library)) can be used with the following format:

```
instance_name
n
W
w_1 d_1
w_2 d_2
...
w_n d_n
```

## Results

<!-- results-start -->
| Instance | Compact LP | Time (s) | ColGen LP | Time (s) | Gap (%) |
| --- | ---: | ---: | ---: | ---: | ---: |
| CSTR10b50c4p10.dat | 101.039 | 0.00310302 | 101.268 | 0.174799 | 0.23 |
| CSTR10b50c4p11.dat | 134.775 | 0.00476003 | 135.583 | 0.100368 | 0.60 |
| CSTR10b50c4p12.dat | 176.229 | 0.00637579 | 206.833 | 0.00727288 | 14.80 |
| CSTR10b50c4p13.dat | 98.2139 | 0.00862789 | 98.4792 | 0.477804 | 0.27 |
| CSTR10b50c4p14.dat | 127.799 | 0.00355196 | 129.25 | 0.0785021 | 1.12 |
| CSTR10b50c4p15.dat | 135.64 | 0.00444913 | 136.625 | 0.240941 | 0.72 |
| CSTR10b50c4p16.dat | 149.672 | 0.012279 | 153.133 | 0.0793935 | 2.26 |
| CSTR10b50c4p17.dat | 181.402 | 0.0131021 | 190.75 | 0.0185929 | 4.90 |
| CSTR10b50c4p18.dat | 164.89 | 0.0127978 | 167.867 | 0.119 | 1.77 |
| CSTR10b50c4p19.dat | 127.54 | 0.0103199 | 127.918 | 0.330937 | 0.30 |
| CSTR10b50c4p1.dat | 173.348 | 0.006284 | 180.667 | 0.0284318 | 4.05 |
| CSTR10b50c4p20.dat | 113.881 | 0.0068481 | 114.381 | 0.346641 | 0.44 |
| CSTR10b50c4p2.dat | 150.259 | 0.00419211 | 152.5 | 0.0236002 | 1.47 |
| CSTR10b50c4p3.dat | 146.455 | 0.00402713 | 150.75 | 0.0343982 | 2.85 |
| CSTR10b50c4p4.dat | 124.019 | 0.00677204 | 124.946 | 0.278829 | 0.74 |
| CSTR10b50c4p5.dat | 77.927 | 0.0042901 | 77.9846 | 0.479443 | 0.07 |
| CSTR10b50c4p6.dat | 118.003 | 0.00320506 | 119.429 | 0.0354928 | 1.19 |
| CSTR10b50c4p7.dat | 135.729 | 0.00391102 | 136.89 | 0.0870377 | 0.85 |
| CSTR10b50c4p8.dat | 158.961 | 0.0120671 | 167.917 | 0.0179008 | 5.33 |
| CSTR10b50c4p9.dat | 134.419 | 0.010632 | 135.526 | 0.135663 | 0.82 |
<!-- results-end -->

$$\text{Gap} = \frac{z_{\text{CG}}^{\text{LP}} - z_{\text{Compact}}^{\text{LP}}}{z_{\text{CG}}^{\text{LP}}} \times 100$$
