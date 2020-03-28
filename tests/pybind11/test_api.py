__author__           = "Dilawar Singh"
__copyright__        = "Copyright 2019-, Dilawar Singh"
__maintainer__       = "Dilawar Singh"
__email__            = "dilawars@ncbs.res.in"

import moose

def test_children():
    a1 = moose.Neutral('/a')
    a2 = moose.Neutral('/a/b')
    moose.Neutral('/a/b/c1')
    moose.Neutral('/a/b/c2')
    assert len(a1.children) == 1
    assert len(a2.children) == 2
    print(a1, a2)

def main():
    test_children()

if __name__ == '__main__':
    main()

