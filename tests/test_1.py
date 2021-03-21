import pytest
import inspect
import numpy as np
import networkx as nx
from precedenceConstrainedKnapsack import precedenceConstrainedKnapsack

# @pytest.mark.parametrize("lpRelax", [True, False])
@pytest.mark.parametrize("lpRelax", [True])
def test_1(lpRelax):
    weight = [1, 2, 3]
    profit = [1, 4, 1]
    edges = []

    solution = precedenceConstrainedKnapsack(
        profit, weight, edges, maxWeight=0, lpRelax=lpRelax
    )
    assert solution == ([0, 0, 0], 0)

    solution = precedenceConstrainedKnapsack(
        profit, weight, edges, maxWeight=2.1, lpRelax=lpRelax
    )
    assert solution == ([0, 1, 0], 0)

    solution = precedenceConstrainedKnapsack(
        profit, weight, edges, maxWeight=3, lpRelax=lpRelax
    )
    assert solution == ([1, 1, 0], 0)

    solution = precedenceConstrainedKnapsack(
        profit, weight, edges, maxWeight=6, lpRelax=lpRelax
    )
    assert solution == ([1, 1, 1], 0)


def test_2():
    assert inspect.getdoc(precedenceConstrainedKnapsack)


@pytest.mark.parametrize("n, seed", [(5000, 0)])
def test_3(n, seed):
    random = np.random.RandomState(seed)

    weight = random.random(n)
    maxWeight = sum(weight) / 2  # arbitrary
    profit = random.random(n)
    edges = list(nx.random_tree(n=n, seed=seed).edges())

    solution, code = precedenceConstrainedKnapsack(
        profit, weight, edges, maxWeight=maxWeight, lpRelax=True
    )
    assert code == 0

    solutionWeight = np.dot(solution, weight)
    assert solutionWeight < maxWeight
