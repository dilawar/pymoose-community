import moose._moose as M
print("Using _moose from %s" % M.__file__)
print(dir(M))
a1 = M.create('a1', 'Neutral', 1)
print(a1)
a2 = M.create('a2', 'Neutral', 1)
print(a2)
