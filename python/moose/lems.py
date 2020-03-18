# -*- coding: utf-8 -*-

# THIS IS IN ALPHA. SHOULD NOT EVEN BE TRIED BY OTHER THAN MAIN DEVELOPER.
# LEMS support in MOOSE simulator.
#
# REFERENCES:
# ----------
#  - Docuementation: http://lems.github.io/LEMS/
#  - Paper: https://doi.org/10.3389/fninf.2014.00079

__author__ = "Dilawar Singh"
__copyright__ = "Copyright 2019-, Dilawar Singh"
__maintainer__ = "Dilawar Singh"
__email__ = "dilawars@ncbs.res.in"

import sys
assert sys.version_info >= (3, 6), "Minimum Python version 3.6 is required."

import time
import typing as _T
from pathlib import Path

import moose
from moose.neuroml2.units import SI

import lxml.etree as ET

import logging
logger_ = logging.getLogger('moose.LEMS')
logger_.setLevel(logging.INFO)

SCRIPT_DIR = Path(__file__).parent


def _findInNeuromlCoreTypes(incpath: Path) -> Path:
    """incpath may be a Neuroml2CoreType. These are included in the source code
    now. 

    FIXME: Ideally they should be distributed in pyneuroml library. 
    See: https://github.com/NeuralEnsemble/libNeuroML/issues/88
    """
    global SCRIPT_DIR
    nmlCoreTypePath = SCRIPT_DIR / 'neuroml2' / 'NeuroML2CoreTypes'
    if (nmlCoreTypePath / incpath.name).exists():
        return nmlCoreTypePath / incpath.name
    return incpath


def _dumpXML(xml, fs=sys.stdout) -> None:
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

        logger_.debug(f"Replacing Include {includeFilePath} by its content.")

        # If the included file has extenstion `nml`, load neuroml.
        if includeFilePath.suffix == '.nml':
            logger_.debug(f"Loading neuroml2 model {includeFilePath}")
            moose.mooseReadNML2(str(includeFilePath), verbose=True)
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


def _printMooseObject():
    for x in moose.wildcardFind('/##'):
        if '/classes' == x.path[:8] or '/Msgs' == x.path[:5]:
            continue
        print(f" {x.path:50s} {type(x)}")


def _quantityToMooseElems(path, prefix='/model'):
    # The last value is field value.
    path = Path(path).parent
    return moose.element(prefix + '/' + str(path))

def _getPopulation(popid):
    return moose.wilcardFind('%s/##[TYPE=Neuron]'%popid)


class LEMS(object):
    def __init__(self, lemsFile: Path, **kwargs):
        self.lemsFile = lemsFile
        self.kwargs = kwargs
        xml = ET.parse(str(lemsFile))
        self.xml = _flattenXML(xml, lemsFile.parent)

        if self.kwargs['debug']:
            outfile = '_flatten.moose-lems.xml'
            with open(outfile, 'w') as f:
                _dumpXML(self.xml, f)
            logger_.debug(f"Wrote flattened model to {outfile}")

    def build(self):
        parent = moose.Neutral('/model')

        # Starting point in LEMS is Target
        for tgt in self.xml.findall('./Target'):
            self.addTarget(tgt, parent)

    def addNetwork(self, network, mroot):
        netID = network.attrib['id']
        logger_.info("Adding network %s to simulation %s" % (netID, mroot))

    def addTable(self, lineElem):
        variable = lineElem.attrib['id']
        assert variable in ['v', 'i']
        quantity = lineElem.attrib['quantity']
        mElem = _quantityToMooseElems(quantity, '/model')
        tab = moose.Table(mElem.path + '/tab')
        moose.connect(tab, 'requestOut', mElem, 'get' + variable.upper())
        return tab

    def addComponentDisplay(self, elem, mroot):
        logger_.info(f"Adding {elem.tag} under simulation component.")
        print(elem.attrib)
        for lElem in elem:
            quantity = lElem.attrib['quantity']
            logger_.info(f" Adding line {lElem.attrib} for {quantity}")
            table = self.addTable(lElem)
            print(table)

    def addSimulationComponent(self, simElem, mRoot):
        if simElem.tag == 'Display':
            self.addComponentDisplay(simElem, mRoot)
        else:
            logger_.warning(f"Not implmented: Component: {simElem.tag}")

    def addSimulation(self, sim, mroot):
        simID = sim.attrib['id']
        logger_.info("Adding simulation %s under %s" % (simID, mroot))
        simTarget = sim.attrib['target']
        assert simTarget, "<Simulation> must have a 'target' attribute"
        for net in sim.xpath("//Network[@id='%s']" % simTarget):
            mroot = moose.Neutral(mroot.path + '/' + simTarget)
            self.addNetwork(net, mroot)

        # Find other elements under simulation
        [self.addSimulationComponent(x, mroot) for x in sim]

    def addTarget(self, tgt, mroot):
        cname = tgt.attrib['component']
        assert cname
        logger_.info(f"Adding target {cname} under {mroot}")

        # There can be at most one simulation under target. Let the schema
        # handle the validation.
        for sim in tgt.xpath("//Simulation[@id='%s']" % cname):
            mroot = moose.Neutral(mroot.path + '/' + sim.attrib['id'])
            self.addSimulation(sim, mroot)

            #  _printMooseObject()

            # Now add the simulation time.
            simtime = SI(sim.attrib['length'])
            #  logger_.info("Running for %s s" % simtime)
            #  t0 = time.time()
            moose.reinit()
            moose.start(simtime)
            #  logger_.info("Finished in %f s" % (time.time()-t0) )


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

    parser.add_argument('-I',
                        default=[],
                        action='append',
                        help='include paths.',
                        metavar='<INCLUDE PATH>')

    parser.add_argument('--debug',
                        '-d',
                        default=False,
                        action='store_true',
                        help='Debug mode.')

    class Args:
        pass

    args = Args()
    parser.parse_args(namespace=args)
    main(**vars(args))
