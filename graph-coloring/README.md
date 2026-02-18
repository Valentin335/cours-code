# Graph Coloring

Given an undirected graph $G = (V, E)$, assign a color to each vertex such that no two adjacent vertices share the same color, while minimizing the number of colors used.

This problem is NP-hard in general and appears in applications such as frequency allocation, exam scheduling, and register allocation.

> Course reference: *Integer Programming — méthodes de décomposition* (F. Clautiaux), Cours 1 & TD 1.

## Formulations

| Folder | Approach | Description |
|--------|----------|-------------|
| `compact/` | Compact MILP | Binary variables $x_{v,c}$ and $y_c$ with conflict constraints |
| `column-generation/` | Column generation | Set covering formulation over independent sets, solved via pricing |

## Data

Instance files are stored in `data/`.
