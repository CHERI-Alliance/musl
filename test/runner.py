import os
import json
import subprocess
import time
import re

from os.path import basename
from string import Template
from datetime import datetime
from multiprocessing import Pool as pool
from argparse import ArgumentParser, Namespace

class ArgParser(ArgumentParser):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.command = Namespace()

    def parse(self):
        self.command = self.parse_args()
        return self

    def add(self, *args, **kwargs):
        self.add_argument(*args, **kwargs)
        if 'default' in kwargs and 'dest' in kwargs:
            self.set_defaults(**{kwargs['dest']: kwargs['default']})
        return self

    def flag(self, *args, **kwargs):
        name = args[0][2:]
        self.add_argument('--%s' % name, dest=name, action='store_true', **kwargs)
        self.add_argument('--no-%s' % name, dest=name, action='store_false', **kwargs)
        if 'default' in kwargs:
            self.set_defaults(**{name: kwargs['default']})
        return self

    def get(self, name: str, fmt=lambda u: u):
        if hasattr(self.command, name):
            return fmt(getattr(self.command, name))
        else:
            return None


report = Template('''<?xml version="1.0" encoding="UTF-8"?>
<testsuites>
<testsuite name="${suite}" errors="${errors}" failures="${failures}" skipped="${skipped}" tests="${tests}" hostname="tbd" time="${total}" timestamp="${timestamp}">
${testcases}
</testsuite>
</testsuites>
''')

passcase = Template('''<testcase classname="${suite}" name="${name}" time="${time}" status="run">
<system-out><![CDATA[${stdout}]]></system-out>
<system-err><![CDATA[${stderr}]]></system-err>
</testcase>''')
failcase = Template('''<testcase classname="${suite}" name="${name}" time="${time}" status="run">
<failure message="${name} has failed" type="failure">${description}</failure>
<system-out><![CDATA[${stdout}]]></system-out>
<system-err><![CDATA[${stderr}]]></system-err>
</testcase>''')
skipcase = Template('''<testcase classname="${suite}" name="${name}" time="${time}" status="run">
<skipped/>
<system-out><![CDATA[${stdout}]]></system-out>
<system-err><![CDATA[${stderr}]]></system-err>
</testcase>''')

def run_process(command: list, stdin: str, timeout: int, _env: dict):
    child = subprocess.Popen(command, universal_newlines=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        stdin=subprocess.PIPE,
        env={**_env, **dict(os.environ)})
    stdout, stderr = child.communicate(stdin, timeout=timeout)
    code = child.returncode
    return code, stdout, stderr


def build_test(_tc: dict, cwd: str, _runner: str, _mie_extra_params: str) -> tuple:
    app: str = _tc.get('app').replace('${build}', cwd)
    params: list = _tc.get('params', [])
    args: list = _tc.get('args', [])
    _tname = _tc.get('name', '%s%s' % (basename(app).replace('.', '-'),
        ('-%s' % ('-'.join([str(t) for t in args]))) if args else ''))
    _cmd: list = [_runner] + params + (_mie_extra_params.split(' ')) + ['--', app] + args
    _xrc: list = _tc.get('xrc', [0])
    _xout: list = _tc.get('stdout', [])
    _xerr: list = _tc.get('stderr', [])
    _env: dict = _tc.get('env', {})
    return _tname, _cmd, _xrc, _xout, _xerr, _env

def check_return_code(_rc: int, _xrc: list) -> tuple:
    return _rc in _xrc, 'returned %s as expected' % _rc, 'returned %s instead of any of {%s}' % (_rc, ', '.join([str(t) for t in _xrc]))

def check_output(output: list, expected: list) -> tuple:
    if not expected:
        # success, pass message, failure message
        return None, 'output not checked', ''
    ln = len(output)
    cur = 0
    results = []
    checking = []

    for k, pattern in enumerate(expected):
        checking.append(True)
        try:
            matcher = re.compile(pattern)
        except:
            matcher = None
        while cur < ln:
            line = output[cur].strip()
            if not line:
                cur += 1
                continue
            if checking[-1]:
                _res = pattern in line or (matcher.match(line) if matcher else False)
                if _res:
                    checking[-1] = False
                    cur += 1
                    break
                else:
                    cur += 1
                    continue
            else:
                cur += 1
                break
        if cur >= ln:
            checking[-1] = True

    for item, failed in zip(expected, checking):
        if failed:
            results.append((False, 'cannot find match for `%s`' % item))
    return all([t for t, _msg in results]), \
           'expected output', \
           'incorrect output\n  - %s' % '\n  - '.join(['%s' % _msg for t, _msg in results if not t])

def check_results(_rc: int, _out: list, _err: list, _xrc: list, _xout: list, _xerr: list) -> tuple:
    messages = []
    _res, _msg, error = check_return_code(_rc, _xrc)
    if _res:
        messages.append(_msg)
    else:
        return False, error
    _res, _msg, error = check_output(_out, _xout)
    if _res is None:
        pass
    elif _res:
        messages.append('stdout: %s' % _msg)
    else:
        return False, 'stdout: %s' % error
    _res, _msg, error = check_output(_err, _xerr)
    if _res is None:
        pass
    elif _res:
        messages.append('stderr: %s' % _msg)
    else:
        return False, 'stderr: %s' % error
    return True, '\n - '.join([t for t in messages if t is not None])

# Usage:
# python3 runner.py <cwd> <JSON test spec> <emulator-binary> <suite-name>
#  - cwd: current working directory (it replaces the ${build} placeholder
#              in the JSON test spec)
#  - JSON test spec: description of tests and how to check their results
#  - emulator binary: which emulator executable to use to run the tests
#              (`launcher` means use the launcher from the build tree)
#              (`frontend` means use the `morelloie` frontend from the build tree)
#              (otherwise this must be path to the `morelloie` executable)
#  - suite name: a name of the test suite for the JUnit XML test report

if __name__ == '__main__':

    options = ArgParser() \
        .add('folder', help='current working directory (it replaces the ${build} placeholder in the JSON test spec)') \
        .add('script', help='path to the JSON test spec (description of tests and how to check their results)') \
        .add('emulator', help='path to the emulator binary') \
        .add('suite', help='a name of the test suite for the JUnit XML test report') \
        .add('--only-test', help='name of the test to run', dest='thetest', default=None) \
        .add('--nproc', help='number of processes to run in parallel', dest='nproc', default='8') \
        .add('--mie-args', help='params for emulator', dest='params', default='') \
        .parse()

    # folder -- current working directory
    # script -- path to the JSON spec file
    # how -- which runner to use (path to the `morelloie` executable)
    # suite_name -- name of the test suite for the JUnit XML report
    folder, script, runner, suite_name = options.get('folder'), options.get('script'), options.get('emulator'), options.get('suite')

    # to allow running specific test
    thetest = options.get('thetest')
    nproc: int = options.get('nproc', fmt=lambda t: int(t))
    mie_extra_params = options.get('params')

    with open(script, 'rt') as f:
        suite: list = json.load(f)

    def process(tc):

        tname, cmd, xrc, xout, xerr, env = build_test(tc, folder, runner, mie_extra_params)

        if thetest and tname != thetest:
            # to allow running specific test
            return None

        st = time.time()
        rc, out, err = run_process(cmd, tc.get('stdin', None), int(tc.get('timeout', 5 * 60)), env)
        delta = (time.time() - st)

        res, msg = check_results(rc, out.split('\n'), err.split('\n'), xrc, xout, xerr)
        time_str = '%.3f' % delta
        tsreasons = {}  # why tests are skipped

        if res:
            print('PASSED  %s: %s (%s sec)' % (tname, msg, time_str))
            tres = passcase.substitute(
                name=tname, suite=suite_name.split('-')[0], time=time_str,
                stdout='' if out is None else out,
                stderr='' if err is None else err)
            tskipped = 0
            tfailure = 0
        else:
            skip = tc.get('skip', 'never')
            if skip != 'never':
                if (skip == 'jenkins' and 'JOB_URL' in os.environ) or skip == 'always':
                    tskipped = 1
                    print('SKIPPED (%s: %s): %s (%s sec)' % (tname, skip, msg, time_str))
                    tres = skipcase.substitute(
                        name=tname, suite=suite_name.split('-')[0], time=time_str,
                        stdout='' if out is None else out,
                        stderr='' if err is None else err)
                    skipping = True
                    if skip not in tsreasons:
                        tsreasons[skip] = 0
                    tsreasons[skip] += 1
                else:
                    skipping = False
                    tskipped = 0
                    tres = None
            else:
                skipping = False
                tskipped = 0
                tres = None
            if not skipping:
                tfailure = 1
                print('FAILED  %s: %s (%s sec)' % (tname, msg, time_str))
                tres = failcase.substitute(
                    name=tname, suite=suite_name.split('-')[0], time=time_str,
                    description='Command line: %s' % ' '.join(cmd),
                    stdout='' if out is None else out,
                    stderr='' if err is None else err)
            else:
                tfailure = 0
        return tname, tres, delta, tskipped, tfailure, tsreasons

    if nproc == 1:
        q = [process(t) for t in suite]
    else:
        with pool(nproc) as p:
            q = p.map(process, suite)

    testcases = []  # list of results
    ntests, failures, skipped = 0, 0, 0  # counters for tests
    failed_tests = []
    total = 0.0  # total execution time
    skip_reasons = {}  # why tests are skipped

    for x in q:
        if x is None:
            continue
        stname, sres, stime, sskipped, sfailure, ssreasons = x
        if sres is None:
            continue
        testcases.append(sres)
        ntests += 1
        skipped += sskipped
        failures += sfailure
        if sfailure > 0:
            failed_tests.append(stname)
        total += stime
        for s, n in ssreasons.items():
            if s not in skip_reasons:
                skip_reasons[s] = 0
            skip_reasons[s] += n

    ts = datetime.now()
    with open('suite-%s-results.xml' % suite_name.split('.')[-1], 'wt') as f:
        f.write(report.substitute(
            testcases='\n'.join(testcases),
            suite=suite_name,
            errors='0',
            failures='%s' % failures,
            skipped='%s' % skipped,
            tests='%s' % ntests,
            timestamp=ts.astimezone().strftime('%Y-%m-%dT%H:%M:%S%z'),
            total='%.3f' % total
        ))
    if skipped > 0:
        skipped_info = ': ' + ','.join(['%s=%s' % (k, v) for k, v in skip_reasons.items()])
    else:
        skipped_info = ''
    if failures > 0:
        print('%s of %s tests failed (%s skipped%s)' % (failures, ntests - skipped, skipped, skipped_info))
        print('Failed tests:')
        for tn in failed_tests:
            print('- failed: %s' % tn)
    else:
        print('All %s tests passed (%s skipped%s)' % (ntests - skipped, skipped, skipped_info))
