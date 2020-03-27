import moose._cmoose as M
import numpy as np
import math

def makereac():
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
    sum.setField("numVars", 2)

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
    assert np.isclose(vol, 1e-15), vol
    print("Volume", vol)

    stim = []
    for i in range(100):
        stim.append(vol * M.NA * (1.0 + math.sin(i * 2.0 * M.PI / 100.0)))
    print(stim)

    tab.setField("vector", stim);
    tab.setField("stepSize", 0.0);
    tab.setField("stopTime", 10.0);
    tab.setField("loopTime", 10.0);
    tab.setField("doLoop", True);

    #  Connect outputs
    for i in range(7):
        s.addMsg("Single", M._ObjId(plots.id, i), "requestOut", pools[i], "getConc");

    #  Schedule it.
    for i in range(11, 18):
        s.setClock(i, 0.1);
    s.setClock(18, 0.1);
    return kin 

def test_ksolve():
    import matplotlib.pyplot as plt
    s = M.getShell()
    makereac()
    s.reinit()
    s.start(20.0)
    for tab in M.wildcardFind('/##[TYPE=Table2]'):
        dataN = tab.getNumpy("vector")
        data = tab.getField("vector")
        assert np.allclose(dataN, data)
        # get data.
        print("====", tab.path, dataN.sum(), dataN.mean(), dataN.shape)
        plt.plot(dataN)
        #  print(data)
    plt.savefig(__file__+".test_ksolve.png")
    print("Done ksolve")

def main():
    test_ksolve()

if __name__ == '__main__':
    main()
