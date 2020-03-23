# -*- coding: utf-8 -*-

import time
import functools
import moose._cmoose as _cmoose

import logging
logger_ = logging.getLogger('moose')

# String to python classes.
__classmap__ = {} 

class __MooseClass__(object):
    # type of MOOSE class. Must be set when defining new class dynamically
    # using `type(name, bases, dict)`.
    mType = 'Unknown'

    def __init__(self, path, numData=1, _id=None):
        assert numData > 0
        self._ndata = numData
        self.path = path
        if _id is None:
            _id = _cmoose.create(path, self.mType, numData)
        self.id = _id

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
    cls = type(p.name, (__MooseClass__, ), dict(mType=p.name))
    __addFinfos(cls, cinfo)

    # Add this class to module and save them in a map for easy reuse later.
    __classmap__[p.name] = cls
    setattr(_cmoose, cls.__name__, cls)

logger_.info("Declarting classes took %f sec" % (time.time() - t0))


# Turns C++ object to Python objects.
def __toMooseObject(objid):
    return __classmap__[objid.type](objid.path, _id=objid.id)

def wildcardFind(pattern):
    """wildcardFind.

    Parameters
    ----------
    pattern :
        pattern
    """
    paths = []
    print('xxx', dir(_cmoose))
    for p in _cmoose._wildcardFind(pattern):
        paths.append(__toMooseObject(p))
    return paths


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
