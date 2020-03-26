import moose._cmoose as M
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
    M.set(sum, "numVars", 2)

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
    M.set(A, "concInit", 2);
    M.set(e1Pool, "concInit", 1);
    M.set(e2Pool, "concInit", 1);
    M.set(sum, "expr", "x0+x1");
    M.set(r1, "Kf", 0.2);
    M.set(r1, "Kb", 0.1);
    M.set(r2, "Kf", 0.1);
    M.set(r2, "Kb", 0.0);
    M.set(e1, "Km", 5);
    M.set(e1, "kcat", 1);
    M.set(e1, "ratio", 4);
    M.set(e2, "Km", 5);
    M.set(e2, "kcat", 1);

    vol = M.get(kin, "volume");
    print("Volume", vol)

    stim = []
    for i in range(100):
        stim.append(vol * M.NA * (1.0 + math.sin(i * 2.0 * M.PI / 100.0)))

    M.set(tab, "vector", stim);
    M.set(tab, "stepSize", 0.0);
    M.set(tab, "stopTime", 10.0);
    M.set(tab, "loopTime", 10.0);
    M.set(tab, "doLoop", True);

    #  Connect outputs
    for i in range(7):
        s.addMsg("Single", M._ObjId(plots.id, i), "requestOut", pools[i], "getConc");

    #  Schedule it.
    for i in range(11, 18):
        s.setClock(i, simDt);
    s.setClock(18, plotDt);
    return kin, tab 


def test_shell():
    rec, tab = makereac()
    s = M.getShell()
    s.reinit()
    s.start(20.0)
    print(tab)
    data = M.getNumpy(tab, "vector")
    print(data)
    # get data.
    print("Done ksolve")

if __name__ == '__main__':
    test_shell()
