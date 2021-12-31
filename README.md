__IMP:__ This is fork of https://github.com/BhallaLab/moose-core because the upstream 
didn't respond to various PRs I submitted. To avoid confusion, I have renamed this fork 
to `pymoose-community`. I don't intend to maintain compatibility with upstream.

# About

This is core computational engine of [MOOSE simulator](https://github.com/BhallaLab/moose). This repository contains
C++ codebase and python interface called `pymoose`. For more details about original MOOSE simulator, visit https://moose.ncbs.res.in .

# Installation

We provide python package via `pip`. Note that you may have to uninstall `pymoose` to install this fork.

    $ pip install pymoose-community --user 

To install `nightly` build:

    $ pip install pymoose-community --user --pre --upgrde
    
Have a look at examples, tutorials and demo here https://github.com/dilawar/moose-examples.

# Build 

To build `pymoose`, follow instructions given here at https://github.com/dilawar/pymoose-community/blob/master/INSTALL.md 
