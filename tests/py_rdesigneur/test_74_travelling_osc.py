# This example illustrates molecular transport of an oscillatory reaction
# system, along a closed-end cylinder. The concentrations along the entire 
# length of the cylinder start out uniform. It shows an interesting 
# combination of propagating oscillations and buildup of reagents.
# Author: Upinder Bhalla 
# Converted to pydoc test by Dilawar Singh

import matplotlib
matplotlib.use('Agg')
import moose
import numpy as np
import pylab
import rdesigneur as rd

def test_travelling_osc():
    """Test travelling wave.
    """
    rdes = rd.rdesigneur(
        turnOffElec = True,
        diffusionLength = 2e-6, 
        chemProto = [['makeChemOscillator()', 'osc']],
        chemDistrib = [['osc', 'soma', 'install', '1' ]],
        plotList = [
            ['soma', '1', 'dend/a', 'conc', 'Concentration of a'],
            ['soma', '1', 'dend/b', 'conc', 'Concentration of b'],
            ['soma', '1', 'dend/a', 'conc', 'Concentration of a', 'wave'],
        ],
        moogList = [['soma', '1', 'dend/a', 'conc', 'a Conc', 0, 360 ]]
    )
    a = moose.element( '/library/osc/kinetics/a' )
    b = moose.element( '/library/osc/kinetics/b' )
    s = moose.element( '/library/osc/kinetics/s' )
    a.diffConst = 0
    b.diffConst = 0
    a.motorConst = 1e-6
    rdes.buildModel()
    moose.reinit()
    #  rdes.displayMoogli( 1, 400, rotation = 0, azim = -np.pi/2, elev = 0.0 )
    moose.start(400)
    data = []
    for t in moose.wildcardFind('/##[TYPE=Table2]'): 
        data.append(t.vector)

    m0, u0 = np.mean(data, axis=0), np.std(data, axis=0)
    m1, u1 = np.mean(data, axis=1), np.std(data, axis=1)


    a, b = np.mean(m0), np.std(m0)
    assert np.allclose((0.0016203009180285007, 0.0068698887774955265), (a,b))

    a, b = np.mean(m1), np.std(m1)
    assert np.allclose((0.0016203009180284997, 0.0002795208149288421), (a,b))

    a, b = np.mean(u0), np.std(u0)
    assert np.allclose((0.00038544727505654305, 0.0021265766028024124), (a,b))

    a, b = np.mean(u1), np.std(u1)
    assert(np.allclose((a, b), (0.007060655963556078, 0.0013911415970042662)))
    print('done')
            

if __name__ == '__main__':
    test_travelling_osc()
