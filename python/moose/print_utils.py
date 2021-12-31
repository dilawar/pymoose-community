# -*- coding: utf-8 -*-
# A library with some print functions. Very useful during development.

from __future__ import print_function, division, absolute_import

__author__           = "Dilawar Singh"
__copyright__        = "Copyright 2013, NCBS Bangalore"
__credits__          = ["NCBS Bangalore", "Bhalla Lab"]
__license__          = "GPL"
__version__          = "1.0.0"
__maintainer__       = "Dilawar Singh"
__email__            = "dilawars@ncbs.res.in"
__status__           = "Development"

import moose
import moose.utils as mu

# Following prints the elements in model
def modelInfo(path : str = '/##', **kwargs) -> str:
    """Generate the list of all available moose-elements in model
    """
    mu.info(f"Couting elements in model under {path}")
    msg = []
    types = [ "Table", "Table2", "Compartment", "Pool", "BufPool", "Enz", "Reac" ]
    for t in types:
        paths = moose.wildcardFind("{}[TYPE={}]".format(path, t))
        if len(paths) > 0:
            msg.append("{:>20} : {}".format(t, len(paths)))
    return "\n".join(msg)
