# -*- coding: utf-8 -*-

import os
import matplotlib
matplotlib.use('Agg')

import moose
import sys
from moose.neuroml2.reader import NML2Reader
import numpy as np


def test_run_cell(nogui=False):

    reader = NML2Reader(verbose=True)
    SCRIPT_DIR = os.path.dirname(__file__)

    filename = os.path.join(SCRIPT_DIR, 'nml_files/passiveCell.nml')
    print('Loading: %s' % filename)
    reader.read(filename)

    msoma = reader.getComp(reader.doc.networks[0].populations[0].id, 0, 0)
    print(msoma)

    data = moose.Neutral('/data')

    pg = reader.getInput('pulseGen1')

    inj = moose.Table('%s/pulse' % (data.path))
    moose.connect(inj, 'requestOut', pg, 'getOutputValue')

    vm = moose.Table('%s/Vm' % (data.path))
    moose.connect(vm, 'requestOut', msoma, 'getVm')

    simdt = 1e-6
    plotdt = 1e-4
    simtime = 150e-3

    for i in range(8):
        moose.setClock(i, simdt)
    moose.setClock(8, plotdt)
    moose.reinit()
    moose.start(simtime)

    print("Finished simulation!")

    V = vm.vector
    t = np.linspace(0, simtime, len(V))

    print(V.mean(), V.std())
    I = inj.vector
    print(I.mean(), I.std())

    assert np.allclose((V.mean(), V.std()),
                       (-0.045694303089464726, 0.012196813810727215))

    assert np.allclose((I.mean(), I.std()),
                       (2.6648900732844778e-11, 3.7706075748592135e-11))

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
        plt.savefig(__file__ + '.png')


if __name__ == '__main__':
    nogui = '-nogui' in sys.argv
    test_run_cell(nogui)
