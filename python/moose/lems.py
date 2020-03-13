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

import sys
import moose

from lems.model.model import Model

from pathlib import Path
import xml.etree.ElementTree as ET


import logging
logger_ = logging.getLogger('moose.LEMS')
logger_.setLevel(logging.INFO)

class Args: pass 
args_ = Args()

# namespace support for etree
ns = {'ns' : 'http://www.neuroml.org/lems/0.7.3'}

def addTarget(target, tree, root):
    logger_.info("Adding target '%s'" % target)
    pass


def toMoose(root):
    global args_
    if args_.debug:
        ET.ElementTree(root).write('flatten.xml', encoding='unicode')

    # Everything starts with Taget.
    parent = moose.Neutral('/model')

    for tgt in root.findall('./ns:Target', ns):
        cname = tgt.attrib['component']
        parent = moose.Neutral(parent.path+'/'+cname)
        addTarget(cname, root, parent)

    moose.reinit()


def main():
    global args_
    lemsFile = Path(args_.LEMS)
    assert lemsFile.exists()
    model = Model()
    for idir in args_.I:
        model.add_include_directory(idir)

    # Use lems to parse the file, validate it and return the DOM.
    model.import_from_file(lemsFile)
    model = model.resolve()

    # Its easier to work with ET then with dom. It is not such a costly
    # expression.
    root = ET.fromstring(model.export_to_dom().toxml())
    toMoose(root)


if __name__ == '__main__':
    import argparse
    # Argument parser.
    description = '''Run a LEMS simulation.'''
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('LEMS', help = 'LEMS file.', metavar='<LEMS FILE>')
    parser.add_argument('-I', default=[], action='append'
            , help = 'include paths.', metavar='<INCLUDE PATH>')

    parser.add_argument('--debug', '-d'
             , required = False
             , default=False
             , action='store_true'
             , help = 'Debug mode.'
            )
    parser.parse_args(namespace=args_)
    main()
