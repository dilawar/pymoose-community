# -*- coding: utf-8 -*-

# LEMS support in MOOSE simulator.
#
# REFERENCES:
# ----------
#  - Docuementation: http://lems.github.io/LEMS/
#  - Paper: https://doi.org/10.3389/fninf.2014.00079

__author__     = "Dilawar Singh"
__copyright__  = "Copyright 2019-, Dilawar Singh"
__maintainer__ = "Dilawar Singh"
__email__      = "dilawars@ncbs.res.in"

import sys
assert sys.version_info >= (3,5), "Minimum Python version 3.5 is required."

import time
from pathlib import Path

import moose
from moose.neuroml2.units import SI

import lxml.etree as ET

import logging
logger_ = logging.getLogger('moose.LEMS')
logger_.setLevel(logging.INFO)

SCRIPT_DIR = Path(__file__).parent

# neuroml2 coretypes.
def _findInNeuromlCoreTypes(incpath):
    global SCRIPT_DIR
    nmlCoreTypePath = SCRIPT_DIR / 'neuroml2' / 'NeuroML2CoreTypes'
    if (nmlCoreTypePath / incpath.name).exists():
        return nmlCoreTypePath / incpath.name
    return incpath


def addSimulation(tid, sim, mroot):
    logger_.info("Adding simulation '%s' (%s)" % (tid, mroot))
    # Each target has a component.
    assert sim.attrib['type'] == 'Simulation', "Other type not supported."

    componentToSimulate = sim.attrib['target']
    mSimRoot = moose.Neutral(mroot.path + '/' + componentToSimulate)
    components = sim.xpath("//ns:Component[@id='%s']" % componentToSimulate,
                           namespaces=ns)
    assert components

    moose.reinit()
    runtime = SI(sim.attrib['length'])
    t0 = time.time()
    moose.start(runtime)
    logger_.info("Took %g s for %g sec" % (time.time() - t0, runtime))



def _dumpXML(xml, fs=sys.stdout):
    print(ET.tostring(xml, pretty_print=True).decode('utf8'), file=fs)


def _flattenXML(xml, source_dir):
    """Flatten a given LEMS. All <Include> are replaced by its inner xml.

    :param xml:
    :param source_dir:
    """

    toRemove = []
    for inc in xml.xpath('//Include'):
        includeFilePath = source_dir / inc.attrib['file']

        ## Search them in neuroml core types folder if not found.
        if not includeFilePath.exists():
            includeFilePath = _findInNeuromlCoreTypes(includeFilePath)

        assert includeFilePath.exists(), f"File {includeFilePath} not found"

        # If the included file has extenstion `nml`, load neuroml.
        if includeFilePath.suffix == '.nml':
            moose.mooseReadNML2(str(includeFilePath))
            continue

        # Else replace the included file by its content.
        incParent = inc.getparent()

        # Recursion
        thisXML = ET.parse(str(includeFilePath))
        thisXML = _flattenXML(thisXML, includeFilePath.parent)

        for child in thisXML.getroot():
            incParent.append(child)

        # delete inc node or just ignore it later.
        toRemove.append(inc)

    # Delete all Include nodes. They have been added recursively.
    for x in toRemove:
        xml.getroot().remove(x)

    return xml


class LEMS(object):

    def __init__(self, lemsFile, **kwargs):
        self.lemsFile = lemsFile
        self.kwargs = kwargs
        xml = ET.parse(str(lemsFile))
        self.xml = _flattenXML(xml, lemsFile.parent)

        if self.kwargs['debug']:
            with open('_flatten.xml', 'w') as f:
                _dumpXML(self.xml, f)

    def build(self):
        parent = moose.Neutral('/model')

        # Starting point in LEMS is Target
        for tgt in self.xml.findall('./Target'):
            self.addTarget(tgt, parent)

    def addNetwork(self, network, mroot):
        netID = network.attrib['id']
        logger_.info("Adding network %s to simulation %s" % (netID, mroot))

    def addSimulation(self, sim, mroot):
        simID = sim.attrib['id']
        logger_.info("Adding simulation %s under %s" % (simID, mroot))
        simTarget = sim.attrib['target']
        assert simTarget, "<Simulation> must have a 'target' attribute"
        for net in sim.xpath("//Network[@id='%s']"%simTarget):
            mroot = moose.Neutral(mroot.path+'/'+simTarget)
            self.addNetwork(net, mroot)

    def addTarget(self, tgt, mroot):
        cname = tgt.attrib['component']
        assert cname
        logger_.info("Adding target %s under %s" % (cname, mroot))
        for sim in tgt.xpath("//Simulation[@id='%s']" % cname):
            mroot = moose.Neutral(mroot.path+'/'+sim.attrib['id'])
            self.addSimulation(sim, mroot)

        # Add simulation properties.
        moose.reinit()


def main(**kwargs):
    if kwargs['debug']:
        logger_.setLevel(logging.DEBUG)
    lemsFile = Path(kwargs['LEMS'])
    assert lemsFile.exists()
    lems = LEMS(lemsFile, **kwargs)
    lems.build()


if __name__ == '__main__':
    import argparse
    # Argument parser.
    description = '''Run a LEMS simulation.'''
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('LEMS', help='LEMS file.', metavar='<LEMS FILE>')

    parser.add_argument('-I', default=[], action='append',
                        help='include paths.',
                        metavar='<INCLUDE PATH>')

    parser.add_argument('--debug', '-d', default=False, action='store_true',
                        help='Debug mode.')

    class Args: pass
    args = Args()
    parser.parse_args(namespace=args)
    main(**vars(args))
