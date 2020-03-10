# URL: https://github.com/BhallaLab/moose-core/issues/379
# This issue is consequential only when running many large electrical models
# sequentially. Each model building is slower than the previous one and there
# is a memory leak happening. Even for a simple model like.

import time
import moose
import rdesigneur as rd
import numpy as np
import memprof as M

# pip install memprof
# run it python3 -m memprof

@M.memprof(plot=True)
def main():
    rdes = rd.rdesigneur(
        chanProto = [['make_HH_Na()', 'Na'], ['make_HH_K()', 'K']],
        chanDistrib = [
            ['Na', 'soma', 'Gbar', '1200' ],
            ['K', 'soma', 'Gbar', '360' ]],
        stimList = [['soma', '1', '.', 'inject', '(t>1 && t<2) * 1e-8' ]],
        plotList = [['soma', '1', '.', 'Vm', 'Membrane potential']]
    )
    elecid_ori = rdes.elecid

    for i in np.arange(1, 3000):
        t0 = time.time()
        try:
            moose.delete('/model')
            rdes.elecid = moose.element(elecid_ori)
        except:
            pass
        rdes.buildModel()
        moose.reinit()
        moose.start( 3 )
        print(f"{i}, {time.time()-t0}")

main()
