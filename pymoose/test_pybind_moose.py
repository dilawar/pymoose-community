import _moose as M
print("Using _moose from %s" % M.__file__)
print(dir(M))
a = M.create('a', 'Neutral', 1)
print(a)
