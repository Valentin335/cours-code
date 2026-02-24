#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

struct Instance {
    std::string name;
    int n;
    int W;
    std::vector<int> w; // item sizes
    std::vector<int> d; // item demands
};

// File format:
//   line 1: instance name
//   line 2: number of items n
//   line 3: bar capacity W
//   lines 4..n+3: w_i d_i
inline Instance read_instance(const std::string& path) {
    Instance inst;
    std::ifstream f(path);
    if (!f) {
        std::cerr << "Cannot open " << path << "\n";
        std::exit(1);
    }
    f >> inst.name >> inst.n >> inst.W;
    inst.w.resize(inst.n);
    inst.d.resize(inst.n);
    for (int i = 0; i < inst.n; i++)
        f >> inst.w[i] >> inst.d[i];
    return inst;
}
