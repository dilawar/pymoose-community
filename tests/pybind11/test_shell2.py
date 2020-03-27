import moose._cmoose as M
import numpy as np
import math

def makereac():
    simDt = 0.1
    plotDt = 0.1
    pools = [None]*10
    s = M.getShell()
    print(dir(s))
    kin = s.create("CubeMesh", M._ObjId(), "kinetics", 1)
    tab = s.create("StimulusTable", kin, "tab", 1)
    pools[0] = T = s.create("BufPool", kin, "T", 1);
    pools[1] = A = s.create("Pool", kin, "A", 1);
    pools[2] = B = s.create("Pool", kin, "B", 1);
    pools[3] = C = s.create("Pool", kin, "C", 1);
    pools[4] = D = s.create("Pool", kin, "D", 1);
    pools[5] = E = s.create("Pool", kin, "E", 1);

    # Silly that it has to have this name.
    pools[6] = tot1 = s.create("BufPool", kin, "tot1", 1);

    sum = s.create("Function", tot1, "func", 1);
    sumInput = M._Id(sum.value + 1);
    e1Pool = s.create("Pool", kin, "e1Pool", 1);
    e2Pool = s.create("Pool", kin, "e2Pool", 1);
    e1 = s.create("Enz", e1Pool, "e1", 1);
    cplx = s.create("Pool", e1, "cplx", 1);
    e2 = s.create("MMenz", e2Pool, "e2", 1);
    r1 = s.create("Reac", kin, "r1", 1);
    r2 = s.create("Reac", kin, "r2", 1);
    plots = s.create("Table2", kin, "plots", 7);

    # Connect them up
    tab.connect("output", T, "setN")
    r1.connect("sub", T, "reac")
    r1.connect("sub", A, "reac")
    r1.connect("prd", B, "reac")

    #  Field<unsigned int>::set(sum, "numVars", 2);
    sum.setField("numVars", 2)

    A.connect("nOut", M._ObjId(sumInput, 0, 0), "input")
    B.connect("nOut", M._ObjId(sumInput, 0, 1), "input")
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

    tab.setField("vector", stim);
    tab.setField("stepSize", 0.0);
    tab.setField("stopTime", 10.0);
    tab.setField("loopTime", 10.0);
    tab.setField("doLoop", True);

    #  Connect outputs
    for i in range(7):
        M._ObjId(plots.id, i).connect("requestOut", pools[i], "getConc")

    #  Schedule it.
    for i in range(11, 18):
        s.setClock(i, simDt);
    s.setClock(18, plotDt);
    return kin, tab 

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
