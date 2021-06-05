{ buildPythonPackage
, clang-tools
, pytestCheckHook
, numpy
, networkx
, cbc
, pybind11
}:

buildPythonPackage rec {
  pname = "precedenceConstrainedKnapsack";
  version = "0.1";
  src = ./.;

  nativeBuildInputs = [ clang-tools ];
  buildInputs = [
    cbc
    pybind11
  ];  
  checkInputs = [
    pytestCheckHook
    numpy
    networkx      
  ];
}
