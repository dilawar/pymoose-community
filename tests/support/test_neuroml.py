# -*- coding: utf-8 -*-

from __future__ import absolute_import, print_function, division

__author__           = "Dilawar Singh"
__copyright__        = "Copyright 2015, Dilawar Singh and NCBS Bangalore"
__credits__          = ["NCBS Bangalore"]
__license__          = "GNU GPL"
__version__          = "1.0.0"
__maintainer__       = "Dilawar Singh"
__email__            = "dilawars@ncbs.res.in"
__status__           = "Development"

import sys
import os
import moose
from pathlib import Path

# the model lives in the same directory as the test script
modeldir = Path(__file__).parent

import _neuroml
from _neuroml.FvsI_CA1 import ca1_main, loadModel
from _neuroml.CA1 import loadGran98NeuroML_L123

def test_ca1():
    p = modeldir / '_neuroml'/'cells_channels'/'CA1soma.morph.xml'
    loadModel(p)
    a = ca1_main(4e-13)
    b = ca1_main(8e-13)
    c = ca1_main(14e-13)
    d = ca1_main(18e-13)
    assert (a, b, c, d) == (10, 20, 29, 34), (a, b, c, d)

def test_gran():
    p = modeldir / '_neuroml' / 'CA1soma.net.xml'
    assert loadGran98NeuroML_L123(p) in [8,9]

def main():
    test_ca1()
    test_gran()

if __name__ == '__main__':
    main()
