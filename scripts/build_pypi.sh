#!/bin/bash
set -e

# This creates a package for pip. For testing purpose
python ./setup.wheel.py bdist_wheel

echo "Use twine upload dist/*.whl to upload to PyPI"
