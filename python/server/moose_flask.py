# python REST server using flask.

# DOCS
# Integrate with apache:
# https://flask.palletsprojects.com/en/1.1.x/deploying/mod_wsgi/

import time
import os
import re
import sys
import shutil
import flask
import tempfile
import subprocess
import base64
import moose
import tarfile

from pathlib import Path

from flask import request, jsonify
from flask_cors import CORS

stop_all_ = False
moose_process_ = None
cwd_ = None


# Matplotlib patch for saving each files into separate PNG files.
matplotlibText = """
print( '>>>> saving all figues')
import matplotlib.pyplot as plt
def multipage(filename, figs=None, dpi=200):
    pp = PdfPages(filename)
    if figs is None:
        figs = [plt.figure(n) for n in plt.get_fignums()]
    for fig in figs:
        fig.savefig(pp, format='pdf')
    pp.close()

def saveall(prefix='results', figs=None):
    if figs is None:
        figs = [plt.figure(n) for n in plt.get_fignums()]
    for i, fig in enumerate(figs):
        outfile = '%s.%d.png' % (prefix, i)
        fig.savefig(outfile)
        print( '>>>> %s saved.' % outfile )
    plt.close()

try:
    saveall()
except Exception as e:
    print( '>>>> Error in saving: %s' % e )
    quit(0)
"""

def getstatus():
    global cwd_
    global moose_process_
    if moose_process_ is None or (not cwd_):
        return { 'MOOSE_STATUS': 'STOPPED' }

    stFile = cwd_ / '.moose_status'
    if not stFile.exists():
        return { 'MOOSE_STATUS': 'STOPPED' }

    return stFile.read_text()


def find_files(dirname, ext=None, name_contains=None, text_regex_search=None):
    files = []
    for d, sd, fs in os.walk(dirname):
        for f in fs:
            fpath = os.path.join(d,f)
            include = True
            if ext is not None:
                if f.split('.')[-1] != ext:
                    include = False
            if name_contains:
                if name_contains not in os.path.basename(f):
                    include = False
            if text_regex_search:
                with open(fpath, 'r' ) as f:
                    txt = f.read()
                    if re.search(text_regex_search, txt) is None:
                        include = False
            if include:
                files.append(fpath)
    return files


def bzip_data_to_send(tdir, notTheseFiles = []):
    # Only send new files.
    resdir = tempfile.mkdtemp()
    resfile = os.path.join(resdir, 'results.tar.bz2')
    with tarfile.open(resfile, 'w|bz2') as tf:
        for f in find_files(tdir, ext='png'):
            tf.add(f, os.path.basename(f))

    time.sleep(0.01)
    # now send the tar file back to client
    data = []
    with open(resfile, 'rb' ) as f:
        data = f.read()
        print('Total bytes to send to client: %d' % len(data))
    shutil.rmtree(resdir)
    return data


def images_as_b64(wdir):
    images = find_files(wdir, 'png')
    data = [ ]
    for img in images:
        with open(img, 'rb') as f:
            data.append(base64.b64encode(f.read()))
    return data


def run_simulation_file(post):
    global cwd_, moose_process_
    cwd_ = Path(tempfile.mkdtemp())
    print(f'-> temp {cwd_}')
    data = post.json['content']
    with open(cwd_/'matplotlibrc', 'w') as f:
        f.write('interactive: True\n')
        f.write('backend: Agg\n')

    mainfile = cwd_/'main.py'
    with open(mainfile, 'w') as f:
        f.write(data)
        f.write('\n')
        f.write(matplotlibText)

    t0 = time.time()
    moose_process_ = subprocess.Popen([sys.executable, f.name] , cwd=cwd_)
    moose_process_.communicate()

    t = time.time() - t0
    # bzip = bzip_data_to_send(cwd_)
    # data = base64.b64encode(bzip)
    images = images_as_b64(cwd_);
    return {'status': 'finished', 'time': f'{t:.2f}', 'images': images}


def main(**kwargs):
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

    return app


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
    app = main(vars(args))
    try:
        app.run(host='0.0.0.0', port=args.port)
    except KeyboardInterrupt:
        stop_all_ = True
        quit(1)
