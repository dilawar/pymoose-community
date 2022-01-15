#!/bin/bash -

set -o nounset                              # Treat unset variables as an error
set +e
#rvm get head
brew update || echo "failed to update"
brew outdated cmake || brew install cmake
brew install gsl || brew upgrade gsl
brew install hdf5
brew install python
brew install numpy
brew install boost
brew install doxygen

PYTHON3=$(which python)

brew unlink numpy && brew link numpy || echo "Failed to link numpy"

$PYTHON3 -m pip install pip conan setuptools wheel --upgrade
$PYTHON3 -m pip install matplotlib
$PYTHON3 -m pip install pyNeuroML libNeuroML || echo "Failed to install neuroml"
$PYTHON3 -m pip install scipy
$PYTHON3 -m pip install pylint
$PYTHON3 -m pip install pytest
