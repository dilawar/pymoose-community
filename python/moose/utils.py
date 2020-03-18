# -* coding: utf-8 -*-
# Utility functions for moose.

from __future__ import print_function, division, absolute_import

import types
import parser
import token
import symbol
import math
from datetime import datetime
from collections import defaultdict

import logging
logger_ = logging.getLogger('moose.utils')

import moose._moose as _moose

try:
    from moose.print_utils import *
except Exception as e:
    logger_.warn("print utilities are not loaded due to %s" % e)

try:
    from moose.network_utils import *
except Exception as e:
    logger_.warn("network utilities are not loaded due to %s" % e)

# Print and Plot utilities.
try:
    from moose.plot_utils import *
except Exception as e:
    logger_.warn( "plot utilities are not loaded due to '%s'" % e )


def tree(root, vchar='|', hchar='--', vcount=1, depth=0, prefix='', is_last=False):
    """Pretty-print a MOOSE tree.

    root - the root element of the MOOSE tree, must be some derivatine of Neutral.

    vchar - the character printed to indicate vertical continuation of
    a parent child relationship.

    hchar - the character printed just before the node name

    vcount - determines how many lines will be printed between two
    successive nodes.

    depth - for internal use - should not be explicitly passed.

    prefix - for internal use - should not be explicitly passed.

    is_last - for internal use - should not be explicitly passed.

    """
    root = _moose.element(root)
    # print('%s: "%s"' % (root, root.children))
    for i in range(vcount):
        print(prefix)

    if depth != 0:
        print(prefix + hchar, end=' ')
        if is_last:
            index = prefix.rfind(vchar)
            prefix = prefix[:index] + ' ' * (len(hchar) + len(vchar)) + vchar
        else:
            prefix = prefix + ' ' * len(hchar) + vchar
    else:
        prefix = prefix + vchar

    print(root.name)
    children = []
    for child_vec in root.children:
        try:
            child = _moose.element(child_vec)
            children.append(child)
        except TypeError:
            pass
            # print 'TypeError:', child_vec, 'when converting to element.'
    for i in range(0, len(children) - 1):
        tree(children[i],
                  vchar, hchar, vcount, depth + 1,
                  prefix, False)
    if len(children) > 0:
        tree(children[-1], vchar, hchar, vcount, depth + 1, prefix, True)


def dfs(root, operation, *args):
    """Traverse the tree in a depth-first manner and apply the
    operation using *args. The first argument is the root object by
    default."""
    if hasattr(root, '_visited'):
        return
    operation(root, *args)
    for child in root.children:
        childNode = _moose.Neutral(child)
        dfs(childNode, operation, *args)
    root._visited = True


def stepRun(simtime, steptime, verbose=True, logger=None):
    """Run the simulation in steps of `steptime` for `simtime`."""
    global logger_
    if logger is None:
        logger = logger_
    clock = _moose.element('/clock')
    if verbose:
        msg = 'Starting simulation for %g' % (simtime)
        logger_.info(msg)
    ts = datetime.now()
    while clock.currentTime < simtime - steptime:
        ts1 = datetime.now()
        _moose.start(steptime)
        te = datetime.now()
        td = te - ts1
        if verbose:
            msg = 'Simulated till %g. Left: %g. %g of simulation took: %g s' % (clock.currentTime, simtime - clock.currentTime, steptime, td.days * 86400 + td.seconds + 1e-6 * td.microseconds)
            logger_.info(msg)

    remaining = simtime - clock.currentTime
    if remaining > 0:
        if verbose:
            msg = 'Running the remaining %g.' % (remaining)
            logger_.info(msg)
        _moose.start(remaining)
    te = datetime.now()
    td = te - ts
    dt = min([t for t in _moose.element('/clock').dts if t > 0.0])
    if verbose:
        msg = 'Finished simulation of %g with minimum dt=%g in %g s' % (simtime, dt, td.days * 86400 + td.seconds + 1e-6 * td.microseconds)
        logger_.info(msg)


def resetSim(simpaths, simdt, plotdt, simmethod='hsolve'):
    """Legacy function.
    """
    _moose.reinit()

def setupTable(name, obj, qtyname, tables_path=None, threshold=None, spikegen=None):
    """ Sets up a table with 'name' which stores 'qtyname' field from 'obj'.
    The table is created under tables_path if not None, else under obj.path . """
    if tables_path is None:
        tables_path = obj.path+'/data'
    ## in case tables_path does not exist, below wrapper will create it
    tables_path_obj = _moose.Neutral(tables_path)
    qtyTable = _moose.Table(tables_path_obj.path+'/'+name)
    ## stepMode no longer supported, connect to 'input'/'spike' message dest to record Vm/spiktimes
    # qtyTable.stepMode = TAB_BUF
    if spikegen is None:
        if threshold is None:
            ## below is wrong! reads qty twice every clock tick!
            #moose.connect( obj, qtyname+'Out', qtyTable, "input")
            ## this is the correct method
            _moose.connect( qtyTable, "requestOut", obj, 'get'+qtyname)
        else:
            ## create new spikegen
            spikegen = _moose.SpikeGen(tables_path_obj.path+'/'+name+'_spikegen')
            ## connect the compartment Vm to the spikegen
            _moose.connect(obj,"VmOut",spikegen,"Vm")
            ## spikegens for different synapse_types can have different thresholds
            spikegen.threshold = threshold
            spikegen.edgeTriggered = 1 # This ensures that spike is generated only on leading edge.
    else:
        _moose.connect(spikegen,'spikeOut',qtyTable,'input') ## spikeGen gives spiketimes
    return qtyTable

def connectSynapse(compartment, synname, gbar_factor):
    """
    Creates a synname synapse under compartment, sets Gbar*gbar_factor, and attaches to compartment.
    synname must be a synapse in /library of MOOSE.
    """
    synapseid = _moose.copy(_moose.SynChan('/library/'+synname),compartment,synname)
    synapse = _moose.SynChan(synapseid)
    synapse.Gbar = synapse.Gbar*gbar_factor
    synapse_mgblock = _moose.Mstring(synapse.path+'/mgblockStr')
    if synapse_mgblock.value=='True': # If NMDA synapse based on mgblock, connect to mgblock
        mgblock = _moose.Mg_block(synapse.path+'/mgblock')
        compartment.connect("channel", mgblock, "channel")
    else:
        compartment.connect("channel", synapse, "channel")
    return synapse

def printNetTree():
    """ Prints all the cells under /, and recursive prints the cell tree for each cell. """
    root = _moose.Neutral('/')
    for id in root.children: # all subelements of 'root'
        if _moose.Neutral(id).className == 'Cell':
            cell = _moose.Cell(id)
            print("-------------------- CELL : ",cell.name," ---------------------------")
            printCellTree(cell)

def printCellTree(cell):
    """
    Prints the tree under MOOSE object 'cell'.
    Assumes cells have all their compartments one level below,
    also there should be nothing other than compartments on level below.
    Apart from compartment properties and messages,
    it displays the same for subelements of compartments only one level below the compartments.
    Thus NMDA synapses' mgblock-s will be left out.

    FIXME: no lenght cound on compartment.
    """
    for compartmentid in cell.children: # compartments
        comp = _moose.Compartment(compartmentid)
        print("  |-",comp.path, 'l=',comp.length, 'd=',comp.diameter, 'Rm=',comp.Rm, 'Ra=',comp.Ra, 'Cm=',comp.Cm, 'EM=',comp.Em)
        printRecursiveTree(compartmentid, level=2) # for channels and synapses and recursively lower levels

def setup_vclamp(compartment, name, delay1, width1, level1, gain=0.5e-5):
    """
    Sets up a voltage clamp with 'name' on MOOSE 'compartment' object:
    adapted from squid.g in DEMOS (moose/genesis)
    Specify the 'delay1', 'width1' and 'level1' of the voltage to be applied to the compartment.
    Typically you need to adjust the PID 'gain'
    For perhaps the Davison 4-compartment mitral or the Davison granule:
    0.5e-5 optimal gain - too high 0.5e-4 drives it to oscillate at high frequency,
    too low 0.5e-6 makes it have an initial overshoot (due to Na channels?)
    Returns a MOOSE table with the PID output.
    """
    ## If /elec doesn't exists it creates /elec and returns a reference to it.
    ## If it does, it just returns its reference.
    _moose.Neutral('/elec')
    pulsegen = _moose.PulseGen('/elec/pulsegen'+name)
    vclamp = _moose.DiffAmp('/elec/vclamp'+name)
    vclamp.saturation = 999.0
    vclamp.gain = 1.0
    lowpass = _moose.RC('/elec/lowpass'+name)
    lowpass.R = 1.0
    lowpass.C = 50e-6 # 50 microseconds tau
    PID = _moose.PIDController('/elec/PID'+name)
    PID.gain = gain
    PID.tau_i = 20e-6
    PID.tau_d = 5e-6
    PID.saturation = 999.0
    # All connections should be written as source.connect('',destination,'')
    pulsegen.connect('outputSrc',lowpass,'injectMsg')
    lowpass.connect('outputSrc',vclamp,'plusDest')
    vclamp.connect('outputSrc',PID,'commandDest')
    PID.connect('outputSrc',compartment,'injectMsg')
    compartment.connect('VmSrc',PID,'sensedDest')

    pulsegen.trigMode = 0 # free run
    pulsegen.baseLevel = -70e-3
    pulsegen.firstDelay = delay1
    pulsegen.firstWidth = width1
    pulsegen.firstLevel = level1
    pulsegen.secondDelay = 1e6
    pulsegen.secondLevel = -70e-3
    pulsegen.secondWidth = 0.0

    vclamp_I = _moose.Table("/elec/vClampITable"+name)
    vclamp_I.stepMode = TAB_BUF #TAB_BUF: table acts as a buffer.
    vclamp_I.connect("inputRequest", PID, "output")
    vclamp_I.useClock(PLOTCLOCK)

    return vclamp_I

def setup_iclamp(compartment, name, delay1, width1, level1):
    """
    Sets up a current clamp with 'name' on MOOSE 'compartment' object:
    Specify the 'delay1', 'width1' and 'level1' of the current pulse to be applied to the compartment.
    Returns the MOOSE pulsegen that sends the current pulse.
    """
    ## If /elec doesn't exists it creates /elec and returns a reference to it.
    ## If it does, it just returns its reference.
    _moose.Neutral('/elec')
    pulsegen = _moose.PulseGen('/elec/pulsegen'+name)
    iclamp = _moose.DiffAmp('/elec/iclamp'+name)
    iclamp.saturation = 1e6
    iclamp.gain = 1.0
    pulsegen.trigMode = 0 # free run
    pulsegen.baseLevel = 0.0
    pulsegen.firstDelay = delay1
    pulsegen.firstWidth = width1
    pulsegen.firstLevel = level1
    pulsegen.secondDelay = 1e6 # to avoid repeat
    pulsegen.secondLevel = 0.0
    pulsegen.secondWidth = 0.0
    pulsegen.connect('output',iclamp,'plusIn')
    iclamp.connect('output',compartment,'injectMsg')
    return pulsegen

def get_matching_children(parent, names):
    """ Returns non-recursive children of 'parent' MOOSE object
    with their names containing any of the strings in list 'names'. """
    matchlist = []
    for childID in parent.children:
        child = _moose.Neutral(childID)
        for name in names:
            if name in child.name:
                matchlist.append(childID)
    return matchlist

def underscorize(path):
    """ Returns: / replaced by underscores in 'path'.
    But async13 branch has indices in the path like [0],
    so just replacing / by _ is not enough,
    should replace [ and ] also by _ """
    return path.replace('/','_').replace('[','-').replace(']','-')

def blockChannels(cell, channel_list):
    """
    Sets gmax to zero for channels of the 'cell' specified in 'channel_list'
    Substring matches in channel_list are allowed
    e.g. 'K' should block all K channels (ensure that you don't use capital K elsewhere in your channel name!)
    """
    for compartmentid in cell.children: # compartments
        comp = _moose.Compartment(compartmentid)
        for childid in comp.children:
            child = _moose.Neutral(childid)
            if child.className in ['HHChannel', 'HHChannel2D']:
                chan = _moose.HHChannel(childid)
                for channame in channel_list:
                    if channame in chan.name:
                        chan.Gbar = 0.0

def get_child_Mstring(mooseobject,mstring):
    for child in mooseobject.children:
        if child.className=='Mstring' and child.name==mstring:
            child = _moose.Mstring(child)
            return child
    return None

def connect_CaConc(compartment_list, temperature=None):
    """ Connect the Ca pools and channels within each of the compartments in compartment_list
     Ca channels should have a child Mstring named 'ion' with value set in MOOSE.
     Ca dependent channels like KCa should have a child Mstring called 'ionDependency' with value set in MOOSE.
     Call this only after instantiating cell so that all channels and pools have been created. """
    for compartment in compartment_list:
        caconc = None
        for child in compartment.children:
            neutralwrap = _moose.Neutral(child)
            if neutralwrap.className == 'CaConc':
                caconc = _moose.CaConc(child)
                break
        if caconc is not None:
            child = get_child_Mstring(caconc,'phi')
            if child is not None:
                caconc.B = float(child.value) # B = phi by definition -- see neuroml 1.8.1 defn
            else:
                ## B has to be set for caconc based on thickness of Ca shell and compartment l and dia,
                ## OR based on the Mstring phi under CaConc path.
                ## I am using a translation from Neuron for mitral cell, hence this method.
                ## In Genesis, gmax / (surfacearea*thick) is set as value of B!
                caconc.B = 1 / (2*FARADAY) / \
                    (math.pi*compartment.diameter*compartment.length * caconc.thick)
            for child in compartment.children:
                neutralwrap = _moose.Neutral(child)
                if neutralwrap.className == 'HHChannel':
                    channel = _moose.HHChannel(child)
                    ## If child Mstring 'ion' is present and is Ca, connect channel current to caconc
                    for childid in channel.children:
                        # in async13, gates which have not been created still 'exist'
                        # i.e. show up as a child, but cannot be wrapped.
                        try:
                            child = _moose.element(childid)
                            if child.className=='Mstring':
                                child = _moose.Mstring(child)
                                if child.name=='ion':
                                    if child.value in ['Ca','ca']:
                                        _moose.connect(channel,'IkOut',caconc,'current')
                                        #print 'Connected IkOut of',channel.path,'to current of',caconc.path
                                ## temperature is used only by Nernst part here...
                                if child.name=='nernst_str':
                                    nernst = _moose.Nernst(channel.path+'/nernst')
                                    nernst_params = child.value.split(',')
                                    nernst.Cout = float(nernst_params[0])
                                    nernst.valence = float(nernst_params[1])
                                    nernst.Temperature = temperature
                                    _moose.connect(nernst,'Eout',channel,'setEk')
                                    _moose.connect(caconc,'concOut',nernst,'ci')
                                    #print 'Connected Nernst',nernst.path
                        except TypeError:
                            pass

                if neutralwrap.className == 'HHChannel2D':
                    channel = _moose.HHChannel2D(child)
                    ## If child Mstring 'ionDependency' is present, connect caconc Ca conc to channel
                    for childid in channel.children:
                        # in async13, gates which have not been created still 'exist'
                        # i.e. show up as a child, but cannot be wrapped.
                        try:
                            child = _moose.element(childid)
                            if child.className=='Mstring' and child.name=='ionDependency':
                                child = _moose.Mstring(child)
                                if child.value in ['Ca','ca']:
                                    _moose.connect(caconc,'concOut',channel,'concen')
                                    #print 'Connected concOut of',caconc.path,'to concen of',channel.path
                        except TypeError:
                            pass

