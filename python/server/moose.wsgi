# make sure python3 can find moose.

# Move this file to /var/www/html/ etc. so that apache can find it.
# import sys
# sys.path.insert(0, '/path/to/moose')

import sys
import os
scpath = os.path.dirname(__file__)

sys.path.insert(0, scpath)
sys.path.insert(0, os.path.join(scpath, '..'))

from moose_flask import main

# it must be named application.
application = main()
