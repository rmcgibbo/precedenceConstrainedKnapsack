from distutils.core import Extension, setup
from glob import glob

import pybind11
from setuptools import setup

compile_args = f"""
-std=c++17
-Wall -Wextra -Wformat=2 -Wno-format-nonliteral -Wshadow
-Wpointer-arith -Wcast-qual -Wmissing-prototypes -Wno-missing-braces
-isystem {pybind11.get_include()}
"""

ext_modules = [
    Extension(
        "precedenceConstrainedKnapsack",
        ["src/solver.cxx"],
        language="c++",
        extra_compile_args=compile_args.split(),
        libraries=["OsiClp", "Cbc", "CoinUtils"],
    )
]

setup(
    name="precedenceConstrainedKnapsack",
    version="0.1",
    ext_modules=ext_modules,
    setup_requires=["pybind11"],
)
