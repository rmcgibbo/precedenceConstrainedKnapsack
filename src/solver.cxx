#include <algorithm>
#include <numeric>
#include <sstream>
#include <vector>

#include <coin/CbcModel.hpp>
#include <coin/CoinPackedMatrix.hpp>
#include <coin/CoinPackedVector.hpp>
#include <coin/OsiClpSolverInterface.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;

std::pair<std::vector<int32_t>, int> precedenceConstrainedKnapsack(
    const std::vector<double>& profit,
    const std::vector<double>& weight,
    const std::vector<std::pair<int32_t, int32_t>>& edges,
    double maxWeight,
    double maximumSeconds,
    int numThreads,
    int logLevel,
    double allowableFractionGap,
    bool lpRelax)
{
    OsiClpSolverInterface si;
    if (weight.size() != profit.size()) {
        std::stringstream ss;
        ss << "weight and profit vectors must be the same length. ";
        ss << "weight: size=";
        ss << weight.size();
        ss << " profit: size=";
        ss << profit.size();
        throw std::invalid_argument(ss.str());
    }
    std::vector<double> lb(weight.size(), 0);
    std::vector<double> ub(weight.size(), 1);

    std::vector<double> row_lb;
    std::vector<double> row_ub;

    std::vector<int> matrixRowIndices;
    std::vector<int> matrixColumnIndices;
    std::vector<double> matrixElements;

    for (size_t rowK = 0; rowK < edges.size(); rowK++) {
        auto& edge = edges[rowK];
        if (edge.first < 0 || edge.first >= static_cast<int32_t>(weight.size())) {
            throw std::invalid_argument("Invalid edge index");
        }
        if (edge.second < 0 || edge.second >= static_cast<int32_t>(weight.size())) {
            throw std::invalid_argument("Invalid edge index");
        }

        matrixRowIndices.push_back(rowK);
        matrixColumnIndices.push_back(edge.second);
        matrixElements.push_back(1.0);

        matrixRowIndices.push_back(rowK);
        matrixColumnIndices.push_back(edge.first);
        matrixElements.push_back(-1.0);

        row_lb.push_back(0);
        row_ub.push_back(si.getInfinity());
    }

    CoinPackedMatrix matrix(false,
        &matrixRowIndices[0],
        &matrixColumnIndices[0],
        &matrixElements[0],
        matrixRowIndices.size());

    {
        CoinPackedVector row;
        for (size_t i = 0; i < weight.size(); i++) {
            row.insert(i, weight[i]);
        }
        row_lb.push_back(0);
        row_ub.push_back(maxWeight);
        matrix.appendRow(row);
    }

    si.setLogLevel(logLevel);
    si.loadProblem(matrix, &lb[0], &ub[0], &profit[0], &row_lb[0], &row_ub[0]);

    // If we're doing the LP relaxation, we don't force the variables to be
    // integral
    if (!lpRelax) {
        for (size_t i = 0; i < weight.size(); i++) {
            si.setInteger(i);
        }
    }

    CbcModel model(si);
    model.setLogLevel(logLevel);
    if (maximumSeconds > 0) {
        model.setMaximumSeconds(maximumSeconds);
    }
    if (numThreads > 0) {
        model.setNumberThreads(numThreads);
    }

    model.setAllowableFractionGap(allowableFractionGap);
    model.setObjSense(-1); // maximize

    std::vector<double> hotStart(weight.size(), 0.0);
    model.setHotstartSolution(&hotStart[0]);

    model.branchAndBound();

    int status = model.secondaryStatus();
    const double* cbcSolution = model.bestSolution();
    if (cbcSolution == nullptr) {
        std::stringstream ss;
        ss << "No solution found by ILP solver!";
        ss << " Status Code: ";
        ss << status;
        throw std::length_error(ss.str());
    }
    std::vector<int32_t> solution(weight.size());
    // 0 search completed with solution
    // 1 linear relaxation not feasible (or worse than cutoff)
    // 2 stopped on gap 3 stopped on nodes
    // 4 stopped on time
    // 5 stopped on user event
    // 6 stopped on solutions
    // 7 linear relaxation unbounded
    // 8 stopped on iteration limit.

    // Sort indices (from 0 to len(solution)-1) by the distance
    // their integrality gap.
    std::vector<size_t> inds(weight.size());
    std::iota(inds.begin(), inds.end(), 0);
    std::sort(inds.begin(), inds.end(), [&](size_t i, size_t j) {
        auto gap = [&](size_t k) -> int64_t {
            double value = abs(round(cbcSolution[k]) - cbcSolution[k]);
            return static_cast<int64_t>(round(value * 1e6));
        };
        auto gapi = gap(i);
        auto gapj = gap(j);
        // primary sort is on the integrality gap. but if those are
        // equal we'll break ties by sorting by profit / weight,
        // in reverse, so that high profit-per-weight items are rounded
        // first
        if (gapi == gapj) {
            return (-profit[i] / (weight[i] + 1e-10))
                < (-profit[j] / (weight[j] + 1e-10));
        }
        return gapi < gapj;
    });

    // Round solution in a way that's aware of maxWeight
    auto currentWeight = 0.0;
    for (auto i : inds) {
        bool proposedSolution = round(cbcSolution[i]);
        double proposedWeight = currentWeight + proposedSolution * weight[i];
        if (proposedWeight <= maxWeight) {
            solution[i] = proposedSolution;
            currentWeight = proposedWeight;
        } else {
            solution[i] = 0;
        }
    }

    if (lpRelax) {
        // ensure that the returned solution is feasible according to the
        // constraints

        int numEdgesReset = -1;
        while (numEdgesReset != 0) {
            numEdgesReset = 0;
            for (const auto& edge : edges) {
                // edge.first requires edge.second. So if edge.first is
                // on, but edge.second is not on, then just turn off
                // edge.first.
                if (solution[edge.first] && !solution[edge.second]) {
                    solution[edge.first] = 0;
                    numEdgesReset++;
                }
            }
        }
    }

    return std::make_pair(solution, status);
}

PYBIND11_MODULE(precedenceConstrainedKnapsack, m)
{
    m.def("precedenceConstrainedKnapsack",
        &precedenceConstrainedKnapsack,
        R"(Solve the precedence-constraint knapsack problem.

Quoting the abstract of Samphaiboon & Yamada; J. Opt. Theory Appl. 2000

    > The knapsack problem (KP) is generalized taking into account a
    > precedence relation between items. Such a relation can be represented
    > by means of a directed acyclic graph, where nodes correspond to items
    > in a one-to-one way. As in ordinary KPs, each item is associated with
    > profit and weight, the knapsack has a fixed capacity, and the problem
    > is to determine the set of items to be included in the knapsack.
    > However, each item can be adopted only when all of its predecessors
    > have been included in the knapsack. The knapsack problem with such an
    > additional set of constraints is referred to as the precedence-
    > constrained knapsack problem.

The arguments `profit`, `weight`, and `maxCost` are self explanatory.

The argument `eges` describes the graph. For each edge (i, j), the node `j`
is the predecessor of node `i`. That is, node `i` cannot be included in the
solution unless node `j` is also included in the solution.

`maxSeconds`, if positive, can be used to limit the solver runtime.

`numThreads`, if positive, is passed to the solver to control the number of
threads. Whether or not it is effective depends on how Cbc is compiled.

`logLevel` should be self explanatory.

`allowableFractionGap` can be used, if `lpRelax = false`, to early-stop with
an approximate solution in some cases.

If `lpRelax` is true, we solve the linear program (LP) relaxation, which
is fast and approximate. If `lpRelax` is false, we solve the integer
linear program with is slow for large instances.

The return value is the solution vector (1 for selected items and 0 for unselected
items) as well as the solver status code from Cbc. See the source code for the
interpretation of the solver status codes. 0 is success and the other numbers are
various failure modes.
        )",
        py::arg("profit"),
        py::arg("weight"),
        py::arg("edges"),
        py::arg("maxWeight"),
        py::arg("maxSeconds") = 0,
        py::arg("numThreads") = 0,
        py::arg("logLevel") = 0,
        py::arg("allowableFractionGap") = 0,
        py::arg("lpRelax") = true);
}
