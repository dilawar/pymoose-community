# python REST server using flask.
import matplotlib
matplotlib.use('Agg')

import importlib
import os
import sys
import flask
import moose
import tempfile
from pathlib import Path

from flask import request, jsonify
from flask_cors import CORS

stop_all_ = False

def run_simulation_file(post):
    wdir = tempfile.mkdtemp()
    # with tempfile.TemporaryDirectory(delete=False) as wdir:
    print(f'-> temp {wdir}')
    data = post.json['content']
    try:
        a = exec(data)
        importlib.reload(moose)
        return a
    except Exception as e:
        return str(e)

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
        e = moose.env()
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
