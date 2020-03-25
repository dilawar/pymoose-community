import moose
import moose._cmoose as M

def test_sanity_1():
    print("Using _moose from %s" % M.__file__)
    a0 = M.create('/a0', 'Neutral', 1)
    a1 = M.create('/a0/a1', 'Neutral', 1)
    a2 = M.create('/a2', 'Neutral', 10)
    ps = moose.wildcardFind('/##')
    print("Total elems at statup: %d" % len(ps))
    # Current moose bindings has this number set to 3037
    assert len(ps) <= 3037, len(ps)
    assert len(ps) > 3000, len(ps)

def test_sanity_2():
    print('Current CWE', M.getCwe())
    a = M.create('/xyz', 'Neutral', 1)
    M.setCwe(a)
    print('Current CWE', M.getCwe())

def test_set_get():
    a = M.CubeMesh('a')
    print('a.z1', a.z1)
    print('a.z0', a.z0)
    a.z1 = 0.321123
    a.z0 = 0.123123
    assert a.z0 == 0.123123, a.z0
    assert a.z1 == 0.321123, a.z1
    print(a.z0, a.z1)

def test_sanity_3():
    a = M.Pool('/p1')
    a.concInit = 1e-3
    assert a.concInit == 1e-3, a.concInit
    print(a.concInit)
    print(a)

if __name__ == '__main__':
    test_sanity_1()
    test_sanity_2()
    test_set_get()
    test_sanity_3()
