# -*- coding: utf-8 -*-

# This file is part of MOOSE simulator: http://moose.ncbs.res.in.
# MOOSE is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# MOOSE is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with MOOSE.  If not, see <http://www.gnu.org/licenses/>.

# This script is in beta. It is used to build moose-core as python extension
# module. 
# The aim is to make wheel bdist.

__author__           = "Dilawar Singh"
__copyright__        = "Copyright 2013, Dilawar Singh and NCBS Bangalore"
__credits__          = ["NCBS Bangalore"]
__license__          = "GNU GPL"
__version__          = "1.0.0"
__maintainer__       = "Dilawar Singh"
__email__            = "dilawars@ncbs.res.in"
__status__           = "Development"


import os
import os
import re
import sys
import sysconfig
import platform
import subprocess
import datetime
from distutils.version import LooseVersion
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

buildNo = datetime.datetime.now( ).strftime( '%Y%m%d' )
version = '3.2.%s' % buildNo

script_dir = os.path.dirname( os.path.abspath( __file__ ) )
MOOSE_SRC_DIR_ = os.path.realpath( os.path.join( script_dir, '..' ) )
MOOSE_BUILD_DIR = os.path.join( script_dir, '_build_moose' )

try:
    import importlib.machinery
    suffix = importlib.machinery.EXTENSION_SUFFIXES[0]
except Exception as e:
    suffix = '.so'

# Following two classes CMakeExtension and CMakeBuild is taken from
# http://www.benjack.io/2017/06/12/python-cpp-tests.html
class CMakeExtension(Extension):
    def __init__(self, name, sourcedir='..'):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)

class CMakeBuild(build_ext):

    def run(self):
        try:
            out = subprocess.check_output(['cmake', '--version'])
        except OSError:
            raise RuntimeError(
                "CMake must be installed to build the following extensions: " +
                ", ".join(e.name for e in self.extensions))

        if platform.system() == "Windows":
            cmake_version = LooseVersion(re.search(r'version\s*([\d.]+)',
                                         out.decode()).group(1))
            if cmake_version < '3.1.0':
                raise RuntimeError("CMake >= 3.1.0 is required on Windows")

        for ext in self.extensions:
            self.build_extension(ext)

    def build_extension(self, ext):
        extdir = os.path.abspath(
            os.path.dirname(self.get_ext_fullpath(ext.name)))
        cmake_args = ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + extdir,
                '-DPYTHON_EXECUTABLE=' + sys.executable
                , '-DBUILD_WHEEL=ON'
                ]

        cfg = 'Debug' if self.debug else 'Release'
        build_args = ['--config', cfg]

        if platform.system() == "Windows":
            cmake_args += ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{}={}'.format(
                cfg.upper(),
                extdir)]
            if sys.maxsize > 2**32:
                cmake_args += ['-A', 'x64']
            build_args += ['--', '/m']
        else:
            cmake_args += ['-DCMAKE_BUILD_TYPE=' + cfg]
            build_args += ['--', '-j4']

        env = os.environ.copy()
        env['CXXFLAGS'] = '{} -DVERSION_INFO=\\"{}\\"'.format(
            env.get('CXXFLAGS', ''),
            self.distribution.get_version())
        if not os.path.exists( MOOSE_BUILD_DIR ):
            os.makedirs( MOOSE_BUILD_DIR )
        subprocess.check_call( ['cmake', ext.sourcedir] + cmake_args,
                              cwd=MOOSE_BUILD_DIR, env=env)
        subprocess.check_call( ['cmake', '--build', '.'] + build_args,
                              cwd=MOOSE_BUILD_DIR
                              )
        print()  # Add an empty line for cleaner output


# Setup .
setup(
        name='pymoose', # because moose is not available in pypi.
        version=version,
        description="MOOSE Simulator's Python scripting interface.",
        author='MOOSERes',
        author_email='bhalla@ncbs.res.in',
        maintainer='Dilawar Singh',
        maintainer_email='dilawars@ncbs.res.in',
        url='http://moose.ncbs.res.in',
        packages=[
            'rdesigneur'
            , 'moose'
            , 'moose.SBML'
            , 'moose.neuroml'
            , 'moose.genesis'
            , 'moose.chemUtil'
            , 'moose.chemMerge'
            ],
        package_dir = {
            'moose' : os.path.join( MOOSE_SRC_DIR_, 'python',  'moose' )
            , 'rdesigneur' : os.path.join( MOOSE_SRC_DIR_, 'python', 'rdesigneur' )
            },
        ext_modules = [ CMakeExtension( 'moose' ) ],
        cmdclass = dict( build_ext = CMakeBuild ),
        package_data = { 'moose' : [ '_moose' + suffix ] },
        zip_safe = False
    )
