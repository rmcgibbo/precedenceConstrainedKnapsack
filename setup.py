from glob import glob
from setuptools import setup
import pybind11
from distutils.core import setup, Extension

ext_modules = [
    Extension('precedenceConstrainedKnapsack', ['src/solver.cxx'],
              include_dirs=[pybind11.get_include()],
              language="c++",
              libraries=['OsiClp', 'Cbc', 'CoinUtils'])
]

setup(
    name="precedenceConstrainedKnapsack",
    version="0.1",
    ext_modules=ext_modules,
    setup_requires=["pybind11"],
)
