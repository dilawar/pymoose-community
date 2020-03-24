import moose
import moose._cmoose as M

def test_sanity_1():
    print("Using _moose from %s" % M.__file__)
    a0 = M.create('/a0', 'Neutral', 1)
    a1 = M.create('/a0/a1', 'Neutral', 1)
    a2 = M.create('/a2', 'Neutral', 10)
    for f in moose.wildcardFind('/##'):
        print(f)


def test_sanity_2():
    print(M.getCwe().path)
    a = M.create('/xyz', 'Neutral', 1)
    M.setCwe(a)
    print(M.getCwe().path)

def test_set_get():
    a = M.CubeMesh('a')
    print(a.z1)
    print(dir(a))
    a.z1 = 1.0
    assert a.z1 == 1.0, a.z1

if __name__ == '__main__':
    test_sanity_1()
    test_sanity_2()
    test_set_get()
