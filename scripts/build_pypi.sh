#!/bin/bash
set -e

# This creates a package for pip. For testing purpose
python ./setup.wheel.py bdist_wheel
auditwheel show dist/*.whl

echo "Use twine upload dist/*.whl to upload to PyPI"
echo "Uploading on test repp"
twine upload --repository-url https://test.pypi.org/legacy/ dist/*.whl

echo "To download and install"
pip install --index-url https://test.pypi.org/simple/ --extra-index-url \
    https://pypi.org/simple pymoose --user
