import moose
import moose._cmoose as M
import numpy as np
import math

def makereac():
    simDt = 0.1
    plotDt = 0.1
    pools = [None]*10
    s = M.getShell()
    k = moose.CubeMesh("kinetics", 1)
    t = moose.StimulusTable("/kinetics/StimulusTable")
    pools[0] = T = moose.BufPool(k.path+"/T")
    pools[1] = A = moose.Pool(k.path+"/A")
    pools[2] = B = moose.Pool(k.path + "/B")
    pools[3] = C = moose.Pool(k.path + "/C")
    pools[4] = D = moose.Pool(k.path + "/D")
    pools[5] = E = moose.Pool(k.path + "/E")

    # Silly that it has to have this name.
    pools[6] = tot1 = moose.BufPool(k.path+"/tot1")

    sum = moose.Function(tot1.path + "/func")
    sum.cobj.setField("expr", "x0+x1")

    e1Pool = moose.Pool(k.path + "/e1Pool")
    e2Pool = moose.Pool(k.path + "/e2Pool")
    e1 = moose.Enz(e1Pool.path + "/e1")
    cplx = moose.Pool(e1.path + "/cplx")
    e2 = moose.MMenz(e2Pool.path + "/e2")
    r1 = moose.Reac( k.path + "/r1")
    r2 = moose.Reac(k.path + "/r2")
    plots = moose.Table2(k.path + "/plots", 7)

    # Connect them up
    t.connect("output", T, "setN")
    r1.connect("sub", T, "reac")
    r1.connect("sub", A, "reac")
    r1.connect("prd", B, "reac")

    sum.cobj.setField("numVars", 2)

    A.connect("nOut", sum.cobj.x[0], "input")
    B.connect("nOut", sum.cobj.x[1], "input")
    sum.connect("valueOut", tot1, "setN");

    r2.connect("sub", B, "reac")
    r2.connect("sub", B, "reac")
    r2.connect("prd", C, "reac")

    e1.connect("sub", C, "reac")
    e1.connect("enz", e1Pool, "reac")
    e1.connect("cplx", cplx, "reac")
    e1.connect("prd", D, "reac")

    e2.connect("sub", D, "reac")
    e2Pool.connect("nOut", e2, "enzDest")
    e2.connect("prd", E, "reac")

    # Set parameters.
    A.setField("concInit", 2);
    assert A.getField("concInit") == 2, A.getField("concInit")

    e1Pool.setField("concInit", 1);
    assert e1Pool.getField("concInit") == 1.0

    e2Pool.setField("concInit", 1);
    sum.setField("expr", "x0+x1");
    r1.setField("Kf", 0.2);
    assert r1.getField("Kf") == 0.2
    r1.setField("Kb", 0.1);
    r2.setField("Kf", 0.1);
    r2.setField("Kb", 0.0);
    e1.setField("Km", 5);
    assert e1.getField("Km") ==  5
    e1.setField("kcat", 1);
    e1.setField("ratio", 4);
    e2.setField("Km", 5);
    e2.setField("kcat", 1);

    vol = kin.getField("volume");
    print("Volume", vol)

    stim = []
    for i in range(100):
        stim.append(vol * M.NA * (1.0 + math.sin(i * 2.0 * M.PI / 100.0)))

    t.setField("vector", stim);
    t.setField("stepSize", 0.0);
    t.setField("stopTime", 10.0);
    t.setField("loopTime", 10.0);
    t.setField("doLoop", True);

    #  Connect outputs
    for i in range(7):
        M._ObjId(plots.id, i).connect("requestOut", pools[i], "getConc")

    #  Schedule it.
    for i in range(11, 18):
        s.setClock(i, simDt);
    s.setClock(18, plotDt);
    return kin, t 

def test_ksolve():
    rec, tab = makereac()
    s = M.getShell()
    s.reinit()
    s.start(20.0)
    print(tab)
    dataN = tab.getFieldNumpy("vector")
    data = np.array(tab.getFieldVec("vector"))
    print(data)
    assert np.allclose(dataN, data)
    # get data.
    print("Done ksolve")

def main():
    test_ksolve()

if __name__ == '__main__':
    main()
