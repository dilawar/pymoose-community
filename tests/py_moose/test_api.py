__author__     = "Dilawar Singh"
__copyright__  = "Copyright 2019-, Dilawar Singh"
__maintainer__ = "Dilawar Singh"
__email__      = "dilawars@ncbs.res.in"

import moose
import numpy as np


def test_children():
    a1 = moose.Neutral('/a')
    a2 = moose.Neutral('/a/b')
    a3 = moose.Neutral('/a/b/c1')
    moose.Neutral('/a/b/c2')
    assert len(a1.children) == 1
    assert len(a2.children) == 2
    moose.le(a1)
    moose.le(a2)
    moose.le(a3)
    moose.setCwe(a3)
    s = moose.getCwe()
    assert s == a3, (s, a3)


def test_other():
    a1 = moose.Pool('/ada')
    assert a1.className == 'Pool', a1.className
    finfo = moose.getFieldDict(a1.className)
    s = moose.Streamer('/asdada')
    p = moose.PulseGen('pg1')
    assert p.delay[0] == 0.0
    p.delay[1] = 0.99
    assert p.delay[1] == 0.99, p.delay[1]


def test_vec():
    a = moose.Pool('/p111', 100)
    v = moose.vec(a)
    assert len(v) == 100, len(v)
    assert v == v.vec
    assert v[0] == v.vec[0], (v[0], v.vec[0])


def test_finfos():
    s = moose.SimpleSynHandler('synh')

    s.numSynapses = 10
    assert s.numSynapses == 10

    syns = s.synapse.vec
    print(syns)
    assert len(syns) == 10
    for i, s in enumerate(syns):
        print(s, type(s))
        s.weight = 9.0
    for s in syns:
        assert s.weight == 9.0
    print(syns.weight)

    # this is a shorthand for above for loop.
    syns.weight = 11.121
    assert np.allclose(syns.weight, 11.121), syns.weight

     # try:
     #     print(syns[11])
     # except Exception as e:
     #     print(e, "Great. We must got an exception here")
     # else:
     #     print(syns[11])
     #     raise Exception("This should have failed")

    a = moose.Pool('x13213')
    a.concInit = 0.1
    assert 0.1 == moose.getField(a, 'concInit')

    # Now get some finfos.
    a = moose.element('/classes/Compartment')


def test_inheritance():
    ta = moose.Table2('/tab2', 10)
    tb = moose.wildcardFind('/##[TYPE=Table2]')
    assert len(tb) == len(ta.vec)
    for i, (t1, t2) in enumerate(zip(tb, ta.vec)):
        assert t1 == t2, (t1, t2)
        assert t1.id == t2.id
        assert t1.dataIndex == t2.dataIndex
        assert t1.path == t2.path

    a = moose.CubeMesh('/dadada')
    isinstance(a, moose.CubeMesh)
    assert isinstance(a, moose.CubeMesh)
    aa = moose.wildcardFind('/##[TYPE=CubeMesh]')[0]
    assert a == aa
    # This must be true for isinstance to work.
    assert isinstance(aa, moose.CubeMesh), (a.__class__, aa.__class__)


def test_delete():
    a = moose.Neutral('/xxx')
    b = moose.Neutral('/xxx/1')
    c = moose.Neutral('/xxx/1/2')
    d = moose.Neutral('/xxx/2')
    e = moose.Neutral('/xxx/2/2')
    f = moose.Neutral('/xxx/2/2/3')
    x = moose.wildcardFind('/xxx/##')
    assert len(x) == 5
    moose.delete(e)
    x = moose.wildcardFind('/xxx/##')
    assert len(x) == 3
    moose.delete(a)
    x = moose.wildcardFind('/xxx/##')
    assert len(x) == 0


def test_wrapper():
    a = moose.Pool('/dadadada', concInit=9.99, nInit=10)
    assert a.nInit == 10
    f = moose.Function('/fun1', expr='x0+x1+A+B')
    assert f.expr == 'x0+x1+A+B'
    assert f.numVars == 4, f.numVars


def test_access():
    a1 = moose.Pool('ac1')
    try:
        a2 = moose.Compartment('ac1')
    except Exception:
        pass
    else:
        raise RuntimeError("Should have failed.")
    a2 = moose.element(a1)
    a3 = moose.element(a1.path)
    assert a2 == a3


def main():
    test_children()
    test_finfos()
    test_other()
    test_vec()
    test_delete()
    test_wrapper()
    test_inheritance()
    test_access()


if __name__ == '__main__':
    main()
