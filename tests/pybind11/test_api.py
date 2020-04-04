__author__           = "Dilawar Singh"
__copyright__        = "Copyright 2019-, Dilawar Singh"
__maintainer__       = "Dilawar Singh"
__email__            = "dilawars@ncbs.res.in"

import moose

def test_children():
    a1 = moose.Neutral('/a')
    a2 = moose.Neutral('/a/b')
    a3 = moose.Neutral('/a/b/c1')
    moose.Neutral('/a/b/c2')
    assert len(a1.children) == 1
    assert len(a2.children) == 2
    print(type(a1), a1)
    moose.le(a1)
    moose.le(a2)
    moose.le(a3)
    s = moose.getCwe()
    print(type(s), s)

def test_other():
    print(f"[INFO ] Testing other...")
    a1 = moose.Pool('/ada')
    print('classname', a1.className)
    finfo = moose.getFieldDict(a1.className)
    s = moose.Streamer('/asdada')
    p = moose.PulseGen('pg1')
    print(p.delay, p.delay.type)
    print(p.delay[0])
    p.delay[1] = 0.99
    assert p.delay[1] == 0.99, p.delay[1]

def test_vec():
    a = moose.Pool('/p111', 100)
    v = moose.vec(a)
    print(v, 'vvv')
    assert len(v) == 100, len(v)
    print(v)
    print(v.vec)
    assert v == v.vec
    assert v[0] == v.vec[0], (v[0], v.vec[0])

def test_finfos():
    s = moose.SimpleSynHandler('synh')
    print('synapse.num', s.synapse.num)
    print('synapse.num', s.synapse.num)
    print('synapse.num', s.synapse.num)
    print('synapse: ', s.synapse)
    try:
        print(s.synapse[0])
    except Exception as e:
        print(e, "Must get an exception here")
        pass
    else:
        raise Exception("This should have failed");

    a = moose.Pool('x13213')
    a.concInit = 0.1
    print(moose.getField(a, 'concInit'))

def test_inheritance():
    print("Testing metaclass attributes")
    a = moose.CubeMesh('/dadada')
    isinstance(a, moose.CubeMesh)
    assert isinstance(a, moose.CubeMesh)
    aa = moose.wildcardFind('/##[TYPE=CubeMesh]')[0]
    assert a == aa
    print("a.__class__", a.__class__, type(a.__class__))
    print(a.__class__, type(a.__class__))
    print(aa.__class__, type(aa.__class__))
    #  assert a.__class__ == aa.__class__, (a.__class__, aa.__class__)
    #  print(dir(moose.CubeMesh))
    assert isinstance(aa, moose.CubeMesh), (a.__class__, aa.__class__)

def test_delete():
    print("Testing create/delete")
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
    a = moose.Pool('/dadadada', concInit = 9.99, nInit=10)
    assert a.nInit == 10
    f = moose.Function('/fun1', expr='x0+x1+A+B')
    assert f.expr == 'x0+x1+A+B'
    print(f.numVars)
    assert f.numVars == 4

def main():
    test_children()
    test_finfos()
    test_other()
    test_vec()
    test_inheritance()
    test_delete()
    test_wrapper()

if __name__ == '__main__':
    main()
