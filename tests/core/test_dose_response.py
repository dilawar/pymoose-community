# -*- coding: utf-8 -*-
"""
Makes and plots the dose response curve for bistable models
Author: Sahil Moza
June 26, 2014
Update:
Friday 14 September 2018 05:48:42 PM IST
Tunrned into a light-weight test by Dilawar Singh 

>>> test_dose_response()  # doctest: +NORMALIZE_WHITESPACE
#variable pools 5, rank=3
Totals:    -3273.39    3377.49
gamma =
-1 -1 -2  1  0
 1  1  2  0  1
Nr =
  -1    0    0    1
   0    2   -1   -1
   0    0 -0.5  0.5
LU =
  -1    0    0    1    1    0    0    0    0
   0    2   -1   -1   -1    1    0    0    0
   0    0 -0.5  0.5  0.5  0.5    1    0    0
   0    0    0    0   -1   -1   -2    1    0
   0    0    0    0    1    1    2    0    1
scale=0.100	kcat=0.036
scale=0.158	kcat=0.057
scale=0.251	kcat=0.091
scale=0.398	kcat=0.144
scale=0.631	kcat=0.228
scale=1.000	kcat=0.361
scale=1.585	kcat=0.572
scale=2.512	kcat=0.907
scale=3.981	kcat=1.437
scale=6.310	kcat=2.278
scale=10.000	kcat=3.610
"""

import os
import sys
import moose
import numpy as np

moose.seed(10)

sdir_ = os.path.dirname( os.path.realpath( __file__ ) )
vals_ = [ ]

def setupSteadyState(simdt,plotDt):

    ksolve = moose.Ksolve( '/model/kinetics/ksolve' )
    stoich = moose.Stoich( '/model/kinetics/stoich' )
    stoich.compartment = moose.element('/model/kinetics')
    stoich.ksolve = ksolve
    stoich.path = "/model/kinetics/##"
    state = moose.SteadyState( '/model/kinetics/state' )
    moose.reinit()
    state.stoich = stoich
    state.showMatrices()
    state.convergenceCriterion = 1e-8
    return ksolve, state

def parseModelName(fileName):
    pos1=fileName.rfind('/')
    pos2=fileName.rfind('.')
    directory=fileName[:pos1]
    prefix=fileName[pos1+1:pos2]
    suffix=fileName[pos2+1:len(fileName)]
    return directory, prefix, suffix

# Solve for the steady state
def getState( ksolve, state, vol):
    scale = 1.0 / ( vol * 6.022e23 )
    moose.reinit()
    state.randomInit() # Removing random initial condition to systematically make Dose reponse curves.
    moose.start( 2.0 ) # Run the model for 2 seconds.
    a = moose.element( '/model/kinetics/a' ).conc
    vals_.append( a )
    state.settle()
    
    vector = []
    a = moose.element( '/model/kinetics/a' ).conc
    for x in ksolve.nVec[0]:
        vector.append( x * scale)
    failedSteadyState = any([np.isnan(x) for x in vector])
    if not (failedSteadyState):
        return state.stateType, state.solutionStatus, a, vector


def test_dose_response():
    # Setup parameters for simulation and plotting
    simdt= 1e-2
    plotDt= 1

    # Factors to change in the dose concentration in log scale
    factorExponent = 10  ## Base: ten raised to some power.
    factorBegin = -10
    factorEnd = 11
    factorStepsize = 2
    factorScale = 10.0 ## To scale up or down the factors

    # Load Model and set up the steady state solver.
    # model = sys.argv[1] # To load model from a file.
    model = os.path.join(sdir_, '..', 'data', '19085.cspace')
    modelPath, modelName, modelType = parseModelName(model)
    outputDir = modelPath
    
    modelId = moose.loadModel(model, 'model', 'ee')
    dosePath = '/model/kinetics/b/DabX' # The dose entity

    ksolve, state = setupSteadyState(simdt, plotDt)
    vol = moose.element('/model/kinetics' ).volume
    iterInit = 100
    solutionVector = []
    factorArr = []
    
    enz = moose.element(dosePath)
    init = float(enz.kcat) # Dose parameter
    
    # Change Dose here to .
    for factor in range(factorBegin, factorEnd, factorStepsize ):
        scale = factorExponent ** (factor/factorScale) 
        enz.kcat = init * scale     
        print( "scale={:.3f}\tkcat={:.3f}".format( scale, enz.kcat) )
        for num in range(iterInit):
            stateType, solStatus, a, vector = getState( ksolve, state, vol)
            if solStatus == 0:
                #solutionVector.append(vector[0]/sum(vector))
                solutionVector.append(a)
                factorArr.append(scale)   

    expected = (0.0040, 0.00104)
    got = ( np.mean(vals_), np.std(vals_) )
    assert np.isclose(got, expected, atol=1e-4).all(), "Got %s, expected %s" % (got, expected)
                
    # FIXME: This test occasionally fails on Windows (not sure why)
    joint = np.array([factorArr, solutionVector])
    joint = joint[:,joint[1,:].argsort()]
    got = np.mean( joint ), np.std( joint )
    expected = (20, 900)
    # Close upto 2 decimal place is good enough.
    assert np.allclose(got, expected, rtol=1e-1, atol=1e-1), "Got %s, expected %s" % (got, expected)

if __name__ == '__main__':
    import doctest
    doctest.testmod()
