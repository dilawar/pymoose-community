# python REST server using flask.

import moose
import flask

from flask import request, jsonify
from flask_cors import CORS

stop_all_ = False

async def send_data(websocket, path):
    name = await websocket.recv()
    print(f"< {name}")

    greeting = f"Hello {name}"
    await websocket.send(greeting)
    print(f"> {greeting}")

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

    @app.route('/run/file', methods=['GET', 'POST'])
    def run_file():
        if request.method == 'POST':
            return 'Simulating'
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
