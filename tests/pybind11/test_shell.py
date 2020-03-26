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
    s.addMsg("Single", tab, "output", T, "setN");

    s.addMsg("Single", r1, "sub", T, "reac");
    s.addMsg("Single", r1, "sub", A, "reac");
    s.addMsg("Single", r1, "prd", B, "reac");

    #  Field<unsigned int>::set(sum, "numVars", 2);
    sum.set("numVars", 2)

    s.addMsg("Single", A, "nOut", M._ObjId(sumInput, 0, 0), "input");
    s.addMsg("Single", B, "nOut", M._ObjId(sumInput, 0, 1), "input");
    s.addMsg("Single", sum, "valueOut", tot1, "setN");

    s.addMsg("Single", r2, "sub", B, "reac");
    s.addMsg("Single", r2, "sub", B, "reac");
    s.addMsg("Single", r2, "prd", C, "reac");

    s.addMsg("Single", e1, "sub", C, "reac");
    s.addMsg("Single", e1, "enz", e1Pool, "reac");
    s.addMsg("Single", e1, "cplx", cplx, "reac");
    s.addMsg("Single", e1, "prd", D, "reac");

    s.addMsg("Single", e2, "sub", D, "reac");
    s.addMsg("Single", e2Pool, "nOut", e2, "enzDest");
    s.addMsg("Single", e2, "prd", E, "reac");

    # Set parameters.
    A.set("concInit", 2);
    assert A.get("concInit") == 2, A.get("concInit")

    e1Pool.set("concInit", 1);
    e1Pool.set("concInit", 1)

    e2Pool.set("concInit", 1);
    sum.set("expr", "x0+x1");
    r1.set("Kf", 0.2);
    assert r1.get("Kf") == 0.2
    r1.set("Kb", 0.1);
    r2.set("Kf", 0.1);
    r2.set("Kb", 0.0);
    e1.set("Km", 5);
    assert e1.get("Km") ==  5
    e1.set("kcat", 1);
    e1.set("ratio", 4);
    e2.set("Km", 5);
    e2.set("kcat", 1);

    vol = kin.get("volume");
    print("Volume", vol)

    stim = []
    for i in range(100):
        stim.append(vol * M.NA * (1.0 + math.sin(i * 2.0 * M.PI / 100.0)))

    tab.set("vector", stim);
    tab.set("stepSize", 0.0);
    tab.set("stopTime", 10.0);
    tab.set("loopTime", 10.0);
    tab.set("doLoop", True);

    #  Connect outputs
    for i in range(7):
        s.addMsg("Single", M._ObjId(plots.id, i), "requestOut", pools[i], "getConc");

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
    dataN = tab.getNumpy("vector")
    data = np.array(tab.getVec("vector"))
    assert np.allclose(dataN, data)
    # get data.
    print("Done ksolve")

def main():
    test_ksolve()

if __name__ == '__main__':
    main()
