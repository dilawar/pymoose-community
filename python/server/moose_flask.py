# python REST server using flask.

# DOCS
# Integrate with apache:
# https://flask.palletsprojects.com/en/1.1.x/deploying/mod_wsgi/

import time
import os
import re
import sys
from datetime import datetime
import shutil
import flask
import tempfile
import subprocess
import base64
import moose
import tarfile

from pathlib import Path

from flask import request
from flask_cors import CORS

stop_all_ = False
lines_ = []
moose_process_ = None
cwd_ = Path()


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
    global lines_

    if moose_process_ is None or (not cwd_):
        return { 'MOOSE_STATUS': 'WAITING'
                , '__file__' : moose.__file__
                , 'cwd' : str(cwd_)
                , 'code' : 1
                }

    stFile = cwd_ / '.moose_status'
    if not stFile.exists():
        return {'MOOSE_STATUS': 'WAITING', 'cwd': str(cwd_), 'code':2}

    txt = stFile.read_text().strip()
    # append current line to the text.
    txt = txt[:-1]
    l = '\n'.join(lines_) if lines_ else ''
    lines_ = []
    b64line = base64.b64encode(l.encode())
    txt += ', "b64line" : "' + b64line.decode() + '"}'
    return txt


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
        print('[DEBUG] Total bytes to send to client: %d' % len(data))
    shutil.rmtree(resdir)
    return data


def images_as_b64(wdir):
    images = find_files(wdir, 'png')
    data = []
    for img in images:   
        with open(img, 'rb') as f:
            data.append(base64.b64encode(f.read()).decode())
    return data


def run_simulation_file(post):
    global cwd_, moose_process_, lines_

    cwd_ = Path(tempfile.mkdtemp(prefix='moose'))
    print(f'[DEBUG] Temp {cwd_}')

    data = post.json['content']
    with open(cwd_/'matplotlibrc', 'w') as f:
        f.write('interactive: True\n')
        f.write('backend: Agg\n')

    mainfile = cwd_/'main.py'
    with open(mainfile, 'w') as f:
        f.write(data)
        f.write('\n')
        f.write(matplotlibText)

    # before running MOOSE, chdir to cwd_ so that .moose_status is created in
    # this direcotry.
    wd = os.getcwd()
    os.chdir(cwd_)
    t0 = time.time()
    moose_process_ = subprocess.Popen([sys.executable, f.name]
            , cwd=cwd_
            , stdout=subprocess.PIPE
            , stderr=subprocess.STDOUT
            , env={'PYTHONPATH': os.getenv('PYTHONPATH', '')}
            )
    while True:
        line = moose_process_.stdout.readline().decode('utf8')
        if not line:
            break
        lines_.append(f"{datetime.now()} : {line}")
    t = time.time() - t0
    images = images_as_b64(cwd_);
    os.chdir(wd)
    return {'status': 'finished'
            , 'time': f'{t:.2f}'
            , 'images': images
            , 'output': '\n'.join(lines_)
            }


def main(**kwargs):
    app = flask.Flask(__name__)
    CORS(app)
    app.config['DEBUG'] = True

    @app.route('/', methods=['GET'])
    @app.route('/ping', methods=['GET'])
    def ping():
        return {'alive': True, 'version': moose.version()
                , 'python': sys.executable
                }

    @app.route('/about', methods=['GET'])
    def about():
        return moose.about()

    @app.route('/status', methods=['GET'])
    def status():
        e = getstatus()
        return e

    @app.route('/run/file', methods=['GET', 'POST'])   # deprecated
    @app.route('/simulation/start/file', methods=['GET', 'POST'])
    def run_file():
        if request.method == 'POST':
            return run_simulation_file(request)
        return 'GET'

    @app.route('/simulation/stop/<pid>', methods=['GET', 'POST'])
    def sim_stop(pid):
        return f'NOT IMPLEMNETED: {pid}'

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
    app = main(**vars(args))
    try:
        app.run(host='0.0.0.0', port=args.port)
    except KeyboardInterrupt:
        stop_all_ = True
        quit(1)
