import moose._cmoose as M

def test_sanity_1():
    print("Using _moose from %s" % M.__file__)
    a0 = M.create('/a0', 'Neutral', 1)
    a1 = M.create('/a0/a1', 'Neutral', 1)
    a2 = M.create('/a2', 'Neutral', 10)
    for f in M._wildcardFind('/##'):
        print(f.path)


def test_sanity_2():
    print(M.getCwe().path)
    a = M.create('/xyz', 'Neutral', 1)
    M.setCwe(a)
    print(M.getCwe().path)

if __name__ == '__main__':
    test_sanity_1()
    test_sanity_2()
