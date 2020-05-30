# python REST server using flask.

import time
import os
import sys
import flask
import tempfile
import subprocess
import moose
from pathlib import Path

from flask import request, jsonify
from flask_cors import CORS

stop_all_ = False
moose_process_ = None
cwd_ = None

def getstatus():
    global cwd_
    global moose_process_
    if moose_process_ is None or (not cwd_):
        return { 'MOOSE_STATUS': 'STOPPED' }

    stFile = cwd_ / '.moose_status'
    if not stFile.exists():
        return { 'MOOSE_STATUS': 'STOPPED' }

    return stFile.read_text()


def run_simulation_file(post):
    global cwd_, moose_process_
    cwd_ = Path(tempfile.mkdtemp())
    print(f'-> temp {cwd_}')
    data = post.json['content']
    with open(cwd_/'matplotlibrc', 'w') as f:
        f.write('interactive: True\n')
        f.write('backend: Agg\n')

    with open(cwd_/'main.py', 'w') as f:
        f.write(data)
    t0 = time.time()
    moose_process_ = subprocess.Popen([sys.executable, f.name] , cwd=cwd_)
    moose_process_.communicate()

    t = time.time() - t0
    return {'status': 'finished', 'time': t}


def main(args):
    app = flask.Flask(__name__)
    CORS(app)
    app.config['DEBUG'] = True

    @app.route('/', methods=['GET'])
    def ping():
        return {'alive': True, 'version': moose.version()}

    @app.route('/about', methods=['GET'])
    def about():
        return moose.about()

    @app.route('/status', methods=['GET'])
    def status():
        e = getstatus()
        print('--> env', e)
        return e

    @app.route('/run/file', methods=['GET', 'POST'])
    def run_file():
        if request.method == 'POST':
            return run_simulation_file(request)
        return 'GET'

    app.run(host='0.0.0.0', port=args.port)


if __name__ == '__main__':
    import argparse
    # Argument parser.
    description = '''Run MOOSE server.'''
    parser = argparse.ArgumentParser(description=description, add_help=False)
    parser.add_argument( '--help', action='help', help='Show this msg and exit')
    parser.add_argument('--port', '-p'
        , required = False, default = 31417, type=int
        , help = 'Port number'
        )
    class Args: pass 
    args = Args()
    parser.parse_args(namespace=args)
    try:
        main(args)
    except KeyboardInterrupt as e:
        stop_all_ = True
        quit(1)
