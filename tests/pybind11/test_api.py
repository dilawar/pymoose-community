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
    a1 = moose.Pool('/ada')
    print('classname', a1.className)
    finfo = moose.getFieldDict(a1.className)
    print(finfo)

    s = moose.Streamer('/asdada')

def test_vec():
    a = moose.Pool('/p111', 100)
    v = moose.vec(a)
    assert len(v) == 100, len(v)
    print(v)
    print(v.vec)
    assert v == v.vec
    assert v[0] == v.vec[0], (v[0], v.vec[0])

def main():
    test_children()
    test_other()
    test_vec()

if __name__ == '__main__':
    main()
