#include "gurobi_c++.h"
#include "../../common/instance.hpp"
#include <chrono>
#include <cmath>

using Pattern = std::vector<int>;

// Build the initial restricted master with one trivial pattern per item type.
// Returns the demand constraints (needed to extract duals and add columns).
std::vector<GRBConstr> build_master(const Instance& inst, GRBModel& master) {
    // First pass: create demand constraints (empty for now)
    std::vector<GRBConstr> demand(inst.n);
    for (int i = 0; i < inst.n; i++)
        demand[i] = master.addConstr(GRBLinExpr(0) >= inst.d[i]);
    master.update();

    // Second pass: add one trivial pattern per item type via GRBColumn
    for (int i = 0; i < inst.n; i++) {
        int fill = inst.W / inst.w[i];
        if (fill == 0) {
            std::cerr << "Item " << i << " (w=" << inst.w[i]
                      << ") does not fit in bar (W=" << inst.W << ")\n";
            std::exit(1);
        }
        GRBColumn col;
        col.addTerm(fill, demand[i]);
        master.addVar(0.0, GRB_INFINITY, 1.0, GRB_CONTINUOUS, col);
    }
    master.update();
    return demand;
}

// Add a new column (pattern) to the master problem.
void add_column(GRBModel& master, const std::vector<GRBConstr>& demand,
                const Pattern& pattern) {
    GRBColumn col;
    for (int i = 0; i < static_cast<int>(pattern.size()); i++)
        col.addTerm(pattern[i], demand[i]);
    master.addVar(0.0, GRB_INFINITY, 1.0, GRB_CONTINUOUS, col);
}

// Pricing subproblem (bounded knapsack):
//   max  sum_i pi_i * a_i
//   s.c. sum_i w_i * a_i <= W
//        a_i in {0, ..., floor(W/w_i)}
// Returns (pattern, reduced_cost) where reduced_cost = 1 - pricing_obj.
std::pair<Pattern, double> solve_pricing(const Instance& inst,
                                         const std::vector<double>& pi,
                                         GRBEnv& env) {
    GRBModel model(env);
    model.set(GRB_IntParam_OutputFlag, 0);

    std::vector<GRBVar> a(inst.n);
    GRBLinExpr obj;
    for (int i = 0; i < inst.n; i++) {
        int ub = inst.W / inst.w[i];
        a[i] = model.addVar(0.0, ub, 0.0, GRB_INTEGER);
        obj += pi[i] * a[i];
    }
    model.setObjective(obj, GRB_MAXIMIZE);

    GRBLinExpr cap;
    for (int i = 0; i < inst.n; i++)
        cap += inst.w[i] * a[i];
    model.addConstr(cap <= inst.W);

    model.optimize();

    Pattern pat(inst.n);
    double pricing_obj = model.get(GRB_DoubleAttr_ObjVal);
    for (int i = 0; i < inst.n; i++)
        pat[i] = static_cast<int>(std::round(a[i].get(GRB_DoubleAttr_X)));

    return {pat, 1.0 - pricing_obj};
}

struct ColGenResult {
    double lp_obj;
    double time;
    int n_patterns;
    int n_iterations;
};

ColGenResult solve_column_generation(const Instance& inst) {
    ColGenResult res{};
    const double EPS = 1e-6;
    auto t0 = std::chrono::steady_clock::now();

    GRBEnv env(true);
    env.set(GRB_IntParam_OutputFlag, 0);
    env.start();

    GRBModel master(env);
    master.set(GRB_IntParam_OutputFlag, 0);
    auto demand = build_master(inst, master);

    for (int iter = 1; ; iter++) {
        master.optimize();

        // Extract duals
        std::vector<double> pi(inst.n);
        for (int i = 0; i < inst.n; i++)
            pi[i] = demand[i].get(GRB_DoubleAttr_Pi);

        // Pricing
        auto [new_pat, red_cost] = solve_pricing(inst, pi, env);

        if (red_cost > -EPS) {
            res.lp_obj = master.get(GRB_DoubleAttr_ObjVal);
            res.n_patterns = master.get(GRB_IntAttr_NumVars);
            res.n_iterations = iter;
            break;
        }

        add_column(master, demand, new_pat);
    }

    auto t1 = std::chrono::steady_clock::now();
    res.time = std::chrono::duration<double>(t1 - t0).count();
    return res;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <instance>\n";
        return 1;
    }
    auto inst = read_instance(argv[1]);

    std::cout << "Instance: " << inst.name
              << " (n=" << inst.n << ", W=" << inst.W << ")\n";

    try {
        auto res = solve_column_generation(inst);
        std::cout << "ColGen LP:   " << res.lp_obj
                  << " (" << res.time << "s)\n";
        std::cout << "Patterns:    " << res.n_patterns << "\n";
        std::cout << "Iterations:  " << res.n_iterations << "\n";
    } catch (GRBException& e) {
        std::cerr << "Gurobi error: " << e.getMessage() << "\n";
        return 1;
    }
    return 0;
}
