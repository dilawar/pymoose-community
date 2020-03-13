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
import time
from pathlib import Path

import moose
from moose.neuroml2.units import SI

from lems.model.model import Model

import lxml.etree as ET

import logging
logger_ = logging.getLogger('moose.LEMS')
logger_.setLevel(logging.INFO)

class Args: pass 
args_ = Args()

# namespace support for etree
ns = {'ns' : 'http://www.neuroml.org/lems/0.7.3'}

def addSimulation(tid, sim, mroot):
    logger_.info("Adding simulation '%s' (%s)" % (tid, mroot))
    # Each target has a component.
    assert sim.attrib['type'] == 'Simulation', "Other type not supported."

    componentToSimulate = sim.attrib['target']
    mSimRoot = moose.Neutral(mroot.path + '/' + componentToSimulate) 
    components = sim.xpath("//ns:Component[@id='%s']"%componentToSimulate
            , namespaces=ns)
    assert components
    

    moose.reinit()
    runtime = SI(sim.attrib['length'])
    t0 = time.time()
    moose.start(runtime)
    logger_.info("Took %g s for %g sec" % (time.time()-t0, runtime))


def toMoose(root):
    global args_
    if args_.debug:
        ET.ElementTree(root).write('flatten.xml', pretty_print=True)

    # Everything starts with Taget.
    parent = moose.Neutral('/model')

    for tgt in root.findall('./ns:Target', ns):
        cname = tgt.attrib['component']
        parent = moose.Neutral(parent.path+'/'+cname)
        for sim in root.xpath("//ns:Component[@id='%s']" % cname, namespaces=ns):
            addSimulation(cname, sim, parent)

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
    # expression. Use lxml since children in lxml knows their parent!
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
