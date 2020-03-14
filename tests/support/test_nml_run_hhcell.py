# -*- coding: utf-8 -*-
# run_hhcell.py ---
#
# Filename: run_hhcell.py
# Description:
# Author:
# Maintainer: P Gleeson
# Version:
# URL:
# Keywords:
# Compatibility:
#
#

# Commentary:
#
#
#
#

# Change log:
#
#
#
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street, Fifth
# Floor, Boston, MA 02110-1301, USA.
#
#

# Code:

import os
import matplotlib
matplotlib.use('Agg')
import moose
import sys
from moose.neuroml2.reader import NML2Reader
import numpy as np

SCRIPT_DIR = os.path.dirname(__file__)


def channel_gates():
    """Creates prototype channels under `/library` and plots the time
    constants (tau) and activation (minf, hinf, ninf) parameters for the
    channel gates.

    """
    import matplotlib.pyplot as plt
    lib = moose.Neutral('/library')
    m = moose.element('/library[0]/naChan[0]/gateX')
    h = moose.element('/library[0]/naChan[0]/gateY')
    n = moose.element('/library[0]/kChan[0]/gateX')
    v = np.linspace(n.min,n.max, n.divs+1)

    Z1 = m.tableA / m.tableB
    assert np.allclose((Z1.mean(), Z1.std()), (0.5555422943641506,
        0.45630927943804045))

    Z2 = h.tableA / h.tableB
    print(Z2.mean(), Z2.std())
    assert np.allclose((Z2.mean(), Z2.std()), (0.3530622180892085, 0.446286093435516))
    
    plt.subplot(221)
    plt.plot(v, 1/m.tableB, label='tau_m')
    plt.plot(v, 1/h.tableB, label='tau_h')
    plt.plot(v, 1/n.tableB, label='tau_n')
    plt.legend()
    
    plt.subplot(222)
    plt.plot(v, m.tableA/m.tableB, label='m_inf')
    plt.plot(v, h.tableA/h.tableB, label='h_inf')
    plt.plot(v, n.tableA/n.tableB, label='n_inf')
    plt.legend()
    
    plt.subplot(223)
    plt.plot(v, m.tableA, label='mA(alpha)')
    plt.plot(v, h.tableA, label='hA(alpha)')
    plt.plot(v, n.tableA, label='nA(alpha)')
    plt.legend()
    plt.subplot(224)
    
    plt.plot(v, m.tableB, label='mB')
    plt.plot(v, m.tableB-m.tableA, label='mB-A(beta)')
    
    plt.plot(v, h.tableB, label='hB')
    plt.plot(v, h.tableB-h.tableA, label='hB-A(beta)')
    
    plt.plot(v, n.tableB, label='nB')
    plt.plot(v, n.tableB-n.tableA, label='nB-nA(beta)')
    plt.legend()
    
    plt.savefig(__file__ + '1.png')
    plt.close()


def test_hh_cell(nogui=False):
    
    reader = NML2Reader(verbose=True)

    filename = os.path.join(SCRIPT_DIR, 'nml_files/NML2_SingleCompHHCell.nml')
    print('Loading: %s'%filename)
    reader.read(filename, symmetric=True)
    
    
    msoma = reader.getComp(reader.doc.networks[0].populations[0].id,0,0)
    print(msoma)
    
    
    data = moose.Neutral('/data')
    
    pg = reader.getInput('pulseGen1')
    
    inj = moose.Table('%s/pulse' % (data.path))
    moose.connect(inj, 'requestOut', pg, 'getOutputValue')
    
    
    vm = moose.Table('%s/Vm' % (data.path))
    moose.connect(vm, 'requestOut', msoma, 'getVm')
    
    simdt = 1e-6
    plotdt = 1e-4
    simtime = 300e-3
    #moose.showmsg( '/clock' )
    for i in range(8):
        moose.setClock( i, simdt )
    moose.setClock( 8, plotdt )
    moose.reinit()
    moose.start(simtime)
    
    print("Finished simulation!")
    
    t = np.linspace(0, simtime, len(vm.vector))

    V = vm.vector
    I = inj.vector
    assert np.allclose((V.mean(),V.std()), (-0.0622818195, 0.0152544)), (
            V.mean(), V.std())
    assert np.allclose((I.mean(),I.std()), (2.6657780739753425e-11,
        3.7709218835869634e-11)), (I.mean(), I.std())
    
    if not nogui:
        import matplotlib.pyplot as plt
        plt.subplot(211)
        plt.plot(t, vm.vector * 1e3, label='Vm (mV)')
        plt.legend()
        plt.title('Vm')
        plt.subplot(212)
        plt.title('Input')
        plt.plot(t, inj.vector * 1e9, label='injected (nA)')
        #plt.plot(t, gK.vector * 1e6, label='K')
        #plt.plot(t, gNa.vector * 1e6, label='Na')
        plt.legend()
        plt.figure()
        channel_gates()
        plt.savefig(__file__+'.png')
        
    
if __name__ == '__main__':
    nogui = '-nogui' in sys.argv
    test_hh_cell(nogui)
    
