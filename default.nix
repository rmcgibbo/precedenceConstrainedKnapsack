{ pkgs ? import <nixpkgs> {}
, pythonPackages ? pkgs.python38Packages }:

pythonPackages.buildPythonPackage rec {
    pname = "precedenceConstrainedKnapsack";
    version = "0.1";
    src = ./.;

    buildInputs = with pythonPackages; [ pkgs.cbc pybind11 ];
    checkInputs = with pythonPackages; [ pytestCheckHook ];
}
