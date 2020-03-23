# -*- coding: utf-8 -*-

# Bring everything from c++ module to global namespace.
#  from moose._cmoose import *
import time
import functools
from moose import _cmoose


class MooseClass:
    # type of MOOSE class. Must be set when defining new class dynamically
    # using `type(name, bases, dict)`.
    mType = 'Unknown'

    def __init__(self, path, numData=1):
        assert numData > 0
        self._ndata = numData
        self.path = path
        self.id = _cmoose.create(path, self.mType, numData)

    def __repr__(self):
        return "<moose.%s: id=%d, dataIndex=%d, path=%s>" % (
            self.mType, self.id.value, self._ndata, self.path)


def __addFinfo(cls, finfo, cinfo):
    pass


def __addFinfos(cls, cinfo):
    gets, sets, other = set(), set(), set()
    for x in cinfo.finfoNames:
        if 'get' in x:
            gets.add(x[3:])
        elif 'set' in x:
            sets.add(x[3:])
        else:
            other.add(x)

    # Fields common in both sets and gets are setter and gettter.
    for x in sets & gets:
        propName = x[0].lower() + x[1:]
        # x=x is necessary in lamdas below because of late binding.
        # https://stackoverflow.com/questions/10452770/python-lambdas-binding-to-local-values
        prop = property(lambda obj, x=x: _cmoose.getProperty(obj.id, x)
                , lambda obj, val, x=x: _cmoose.setProperty(obj.id, x, val))
        setattr(cls, propName, prop)


t0 = time.time()
for p in _cmoose._wildcardFind('/##[TYPE=Cinfo]'):
    cinfo = _cmoose.getCinfo(p.name)
    # create a class.
    cls = type(p.name, (MooseClass, ), dict(mType=p.name))
    __addFinfos(cls, cinfo)

    # Add this class to module.
    setattr(_cmoose, cls.__name__, cls)



print("[INFO ] Declarting classes took %f sec" % (time.time() - t0))

# Bring everything from moose.py to global namespace.
# IMP: It will overwrite any c++ function with the same name.  We can override
# some C++ here.
from moose.moose import *
from moose.server import *

# SBML and NML2 support.
from moose.model_utils import *

# C++ core override
from moose.wrapper import *

# Import moose test.
from moose.moose_test import test
