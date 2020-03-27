import moose._cmoose as _cmoose
import moose

def test_func():
    a = moose.Function('/f1')
    a.cobj.setField("expr", "x0+x1")
    assert a.cobj.getField("expr") == "x0+x1", a.cobj.getField("expr")

def main():
    test_func()

if __name__ == '__main__':
    main()
