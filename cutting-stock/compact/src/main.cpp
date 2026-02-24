#include "gurobi_c++.h"
#include "../../common/instance.hpp"

// Compact formulation:
//   min  sum_c y_c
//   s.c. sum_i w_i x_{i,c} <= W y_c   for all c
//        sum_c x_{i,c}     >= d_i      for all i
//        y_c in {0,1}, x_{i,c} in N
//
// Upper bound on bars: C = sum_i d_i.

GRBModel build_model(const Instance& inst, GRBEnv& env, bool relax, int C) {

    GRBModel model(env);

    // y_c: bar c is used
    std::vector<GRBVar> y(C);
    for (int c = 0; c < C; c++)
        y[c] = model.addVar(0.0, 1.0, 1.0,
                            relax ? GRB_CONTINUOUS : GRB_BINARY);

    // x_{i,c}: copies of item i in bar c
    std::vector<std::vector<GRBVar>> x(inst.n, std::vector<GRBVar>(C));
    for (int i = 0; i < inst.n; i++)
        for (int c = 0; c < C; c++)
            x[i][c] = model.addVar(0.0, GRB_INFINITY, 0.0,
                                   relax ? GRB_CONTINUOUS : GRB_INTEGER);

    // Capacity: sum_i w_i x_{i,c} <= W y_c
    for (int c = 0; c < C; c++) {
        GRBLinExpr lhs;
        for (int i = 0; i < inst.n; i++)
            lhs += inst.w[i] * x[i][c];
        model.addConstr(lhs <= inst.W * y[c]);
    }

    // Demand: sum_c x_{i,c} >= d_i
    for (int i = 0; i < inst.n; i++) {
        GRBLinExpr lhs;
        for (int c = 0; c < C; c++)
            lhs += x[i][c];
        model.addConstr(lhs >= inst.d[i]);
    }

    // Symmetry breaking (IP only): y_c >= y_{c+1}
    if (!relax)
        for (int c = 0; c + 1 < C; c++)
            model.addConstr(y[c] >= y[c + 1]);

    return model;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <instance> [time_limit] [--ip]\n";
        return 1;
    }
    double time_limit = 300.0;
    bool solve_ip = false;
    for (int a = 2; a < argc; a++) {
        if (std::string(argv[a]) == "--ip") solve_ip = true;
        else time_limit = std::stod(argv[a]);
    }
    auto inst = read_instance(argv[1]);

    // Trivial upper bound: sum(d_i) (one bar per item copy).
    // Tighter continuous bound: ceil(sum(w_i * d_i) / W).
    // Both yield the same LP relaxation value, but the tighter bound
    // produces a smaller model and solves faster.
    long long total_size = 0;
    for (int i = 0; i < inst.n; i++)
        total_size += static_cast<long long>(inst.w[i]) * inst.d[i];
    int C = static_cast<int>((total_size + inst.W - 1) / inst.W);

    std::cout << "Instance: " << inst.name
              << " (n=" << inst.n << ", W=" << inst.W << ")\n";

    try {
        GRBEnv env(true);
        env.set(GRB_IntParam_OutputFlag, 0);
        env.start();

        // LP relaxation
        auto lp = build_model(inst, env, true, C);
        lp.set(GRB_DoubleParam_TimeLimit, time_limit);
        lp.optimize();
        std::cout << "Compact LP:  " << lp.get(GRB_DoubleAttr_ObjVal)
                  << " (" << lp.get(GRB_DoubleAttr_Runtime) << "s)\n";
        std::cout << "Vars: " << lp.get(GRB_IntAttr_NumVars)
                  << ", Constrs: " << lp.get(GRB_IntAttr_NumConstrs) << "\n";

        // IP (optional)
        if (solve_ip) {
            auto ip = build_model(inst, env, false, C);
            ip.set(GRB_DoubleParam_TimeLimit, time_limit);
            ip.optimize();
            int status = ip.get(GRB_IntAttr_Status);
            if (ip.get(GRB_IntAttr_SolCount) > 0)
                std::cout << "Compact IP:  " << ip.get(GRB_DoubleAttr_ObjVal);
            else
                std::cout << "Compact IP:  no solution";
            std::cout << " (" << ip.get(GRB_DoubleAttr_Runtime) << "s)"
                      << (status == GRB_TIME_LIMIT ? " [time limit]" : "") << "\n";
        }

    } catch (GRBException& e) {
        std::cerr << "Gurobi error: " << e.getMessage() << "\n";
        return 1;
    }
    return 0;
}
