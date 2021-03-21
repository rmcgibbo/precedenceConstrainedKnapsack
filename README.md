# precedenceConstrainedKnapsack
Precedence constrained knapsack problem solver

```
precedenceConstrainedKnapsack(
    profit: List[float],
    weight: List[float],
    edges: List[Tuple[int, int]],
    maxWeight: float,
    maxSeconds: float = 0,
    numThreads: int = 0,
    logLevel: int = 0,
    allowableFractionGap: float = 0,
    lpRelax: bool = True
) -> Tuple[List[int], int]
    
    Solve the precedence-constraint knapsack problem.
    
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
```
