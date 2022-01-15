#!/bin/bash -

#        AUTHOR: Dilawar Singh (), dilawar.s.rajput@gmail.com
#       CREATED: 01/02/2017 10:11:46 AM
#      REVISION:  ---

set -o nounset                              # Treat unset variables as an error
set -e

BUILDDIR=_build_travis

NPROC=3
(
    PYTHON3=$(which python3)

    # Get pylint
    $PYTHON3 -m pip install pylint --user
    $PYTHON3 -m pip install python-libsbml --user
    $PYTHON3 -m pip install pyneuroml --user

    mkdir -p $BUILDDIR && cd $BUILDDIR 
    cmake -DWITH_BOOST_ODE=ON ..
    make -j$NPROC 
    MOOSE_NUM_THREAD=$NPROC ctest -j$NPROC --output-on-failure 
)
set +e
