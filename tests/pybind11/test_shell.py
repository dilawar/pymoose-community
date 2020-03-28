import moose
import moose._cmoose as M
import numpy as np
import math

def makereac0():
    pools = [None]*10
    s = M.getShell()
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
        s.addMsg("Single", M._ObjId(plots.id, i), "requestOut", pools[i], "getConc");

    #  Schedule it.
    for i in range(11, 18):
        s.setClock(i, 0.1);
    s.setClock(18, 0.1);
    return kin 

def makereac1():
    pools = [None]*10
    s = M.getShell()
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
        s.setClock(i, 0.1);
    s.setClock(18, 0.1);
    return kin

def makereac2():
    pools = [None]*10
    kin = moose.CubeMesh("kinetics", 1)
    t = moose.StimulusTable("kinetics/StimulusTable")
    pools[0] = T = moose.BufPool(kin.path+"/T")
    pools[1] = A = moose.Pool(kin.path+"/A")
    pools[2] = B = moose.Pool(kin.path + "/B")
    pools[3] = C = moose.Pool(kin.path + "/C")
    pools[4] = D = moose.Pool(kin.path + "/D")
    pools[5] = E = moose.Pool(kin.path + "/E")

    # Silly that it has to have this name.
    pools[6] = tot1 = moose.BufPool(kin.path+"/tot1")

    sum = moose.Function(tot1.path + "/func")
    #sum.expr = "x0+x1"
     sum.cobj.setField("expr", "x0+x1")

    e1Pool = moose.Pool(kin.path + "/e1Pool")
    e2Pool = moose.Pool(kin.path + "/e2Pool")
    e1 = moose.Enz(e1Pool.path + "/e1")
    cplx = moose.Pool(e1.path + "/cplx")
    e2 = moose.MMenz(e2Pool.path + "/e2")
    r1 = moose.Reac(kin.path + "/r1")
    r2 = moose.Reac(kin.path + "/r2")
    plots = moose.Table2(kin.path + "/plots", 7)

    # Connect them up
    t.connect("output", T, "setN")
    r1.connect("sub", T, "reac")
    r1.connect("sub", A, "reac")
    r1.connect("prd", B, "reac")

    A.connect("nOut", sum.cobj.getElementFieldItem("x", 0), "input")
    B.connect("nOut", sum.cobj.getElementFieldItem("x", 1), "input")
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
    A.cobj.setField("concInit", 2);
    assert A.cobj.getField("concInit") == 2, A.cobj.getField("concInit")

    e1Pool.cobj.setField("concInit", 1);
    assert e1Pool.cobj.getField("concInit") == 1.0

    e2Pool.cobj.setField("concInit", 1);
    r1.cobj.setField("Kf", 0.2);
    assert r1.cobj.getField("Kf") == 0.2
    r1.cobj.setField("Kb", 0.1);
    r2.cobj.setField("Kf", 0.1);
    r2.cobj.setField("Kb", 0.0);
    e1.cobj.setField("Km", 5);
    assert e1.cobj.getField("Km") ==  5
    e1.cobj.setField("kcat", 1);
    e1.cobj.setField("ratio", 4);
    e2.cobj.setField("Km", 5);
    e2.cobj.setField("kcat", 1);

    vol = kin.cobj.getField("volume");
    assert np.isclose(vol, 1e-15), vol

    stim = []
    for i in range(100):
        stim.append(vol * M.NA * (1.0 + math.sin(i * 2.0 * M.PI / 100.0)))

    t.cobj.setField("vector", stim);
    t.cobj.setField("stepSize", 0.0);
    t.cobj.setField("stopTime", 10.0);
    t.cobj.setField("loopTime", 10.0);
    t.cobj.setField("doLoop", True);

    #  Connect outputs
    for i in range(7):
        M._ObjId(plots.cobj.id, i).connect("requestOut", pools[i].cobj, "getConc")

    #  Schedule it.
    for i in range(11, 18):
        moose.setClock(i, 0.1);
    moose.setClock(18, 0.1);
    return kin

def run_and_assert(kin, outfile):
    import matplotlib.pyplot as plt
    s = M.getShell()
    s.reinit()
    s.start(20.0)
    expected = np.array([(1.0003123906444245, 0.7086363600787222)
            , (0.6260566722985899, 0.3917893203877986)
            , (0.5497740344781968, 0.20547049443610566)
            , (0.14214992346023525, 0.056787380715546225)
            , (0.1011146468828446, 0.05852738403146818)
            , (0.1363724056886532, 0.12767635941368843)
            , (1.1726168304388556, 0.43480678058392586)])

    got = []
    for tab in M.wildcardFind('/##[TYPE=Table2]'):
        dataN = tab.getNumpy("vector")
        data = np.array(tab.getField("vector"))
        assert np.allclose(dataN, data)
        plt.plot(data)
        got.append((dataN.mean(), dataN.std()))

    assert np.allclose(expected, np.array(got)), "Expected %s but got %s" % ( 
            expected, got)
    # get data.
    moose.delete(kin)
    plt.savefig(outfile)
    plt.close()
    print("Done ksolve: Saved to %s" % outfile)

def test_ksolve0():
    kin = makereac0()
    run_and_assert(kin, "ksolve0_test0.png")

def test_ksolve1():
    kin = makereac1()
    run_and_assert(kin, "ksolve1_test1.png")

def test_ksolve2():
    print("[INFO ] test 3")
    kin = makereac2()
    run_and_assert(kin, "ksolve1_test2.png")

def main():
    test_ksolve0()
    test_ksolve1()
    test_ksolve2()

if __name__ == '__main__':
    main()
