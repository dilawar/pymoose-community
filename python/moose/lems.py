# -*- coding: utf-8 -*-

# LEMS support in MOOSE simulator.
#
# REFERENCES:
# ----------
#  - Docuementation: http://lems.github.io/LEMS/
#  - Paper: https://doi.org/10.3389/fninf.2014.00079

__author__           = "Dilawar Singh"
__copyright__        = "Copyright 2019-, Dilawar Singh"
__maintainer__       = "Dilawar Singh"
__email__            = "dilawars@ncbs.res.in"

import moose
from pathlib import Path
from lems.model.model import Model
import logging

logger_ = logging.getLogger('moose.LEMS')

class Args: pass 
args_ = Args()

def buildMooseModel(tname, component, parent):
    for child in component.children:
        print(child, child.toxml())


def main():
    global args_
    lemsFile = Path(args_.LEMS)
    assert lemsFile.exists()
    
    model = Model()
    for _dir in args_.I:
        model.add_include_directory(_dir)

    model.import_from_file(lemsFile)
    model = model.resolve()
    moose.Neutral('/model')
    parent = moose.Neutral('/model/LEMS')
    for tname in model.targets:
        logger_.info("Adding target %s" % tname)
        parent = moose.Neutral(parent.path + '/' + tname)
        comps = model.fat_components[tname]
        print(comps)
        buildMooseModel(tname, model.fat_components[tname], parent)


if __name__ == '__main__':
    import argparse
    # Argument parser.
    description = '''Run a LEMS simulation.'''
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('LEMS', help = 'LEMS file.', metavar='<LEMS FILE>')
    parser.add_argument('-I', default=[], action='append'
            , help = 'include paths.', metavar='<INCLUDE PATH>')
    parser.parse_args(namespace=args_)
    main()
