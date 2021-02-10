import pytest
import inspect

from precedenceConstrainedKnapsack import precedenceConstrainedKnapsack


@pytest.mark.parametrize("lpRelax", [True, False])
def test_1(lpRelax):
    weight = [1, 2, 3]
    profit = [1, 4, 1]
    edges = []

    solution = precedenceConstrainedKnapsack(profit, weight, edges, maxCost=0, lpRelax=lpRelax)
    assert solution == ([0, 0, 0], 0)

    solution = precedenceConstrainedKnapsack(profit, weight, edges, maxCost=2.1, lpRelax=lpRelax)
    assert solution == ([0, 1, 0], 0)

    solution = precedenceConstrainedKnapsack(profit, weight, edges, maxCost=3, lpRelax=lpRelax)
    assert solution == ([1, 1, 0], 0)

    solution = precedenceConstrainedKnapsack(profit, weight, edges, maxCost=6, lpRelax=lpRelax)
    assert solution == ([1, 1, 1], 0)


def test_2():
    assert inspect.getdoc(precedenceConstrainedKnapsack)
