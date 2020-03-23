import _cmoose as M
print("Using _moose from %s" % M.__file__)
print(dir(M))
a0 = M.create('/a0', 'Neutral', 1)
a1 = M.create('/a0/a1', 'Neutral', 1)
print(a1)
a2 = M.create('/a2', 'Neutral', 1)
print(a2)

ps = M._wildcardFind('/##[TYPE=Cinfo]')
for p in ps:
    print(p, p.path)
print(len(ps))
