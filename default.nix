{ pkgs ? import <nixpkgs> {}
, python3Packages ? pkgs.python38Packages }:

python3Packages.buildPythonPackage rec {
    pname = "precedenceConstrainedKnapsack";
    version = "0.1";
    src = ./.;

    nativeBuildInputs = [ pkgs.clang-tools ];
    buildInputs = with python3Packages; [
        pkgs.cbc
        pybind11
    ];
    checkInputs = with python3Packages; [
        pytestCheckHook
        numpy
        networkx
    ];
}
