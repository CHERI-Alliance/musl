import os
import sys
import json
import subprocess
import time
import re

from os.path import basename
from string import Template
from datetime import datetime

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
<failure message="${name} has failed" type="failure"></failure>
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


def build_test(_tc: dict, cwd: str, _runner: str, _libtracer: str, _libmie: str) -> tuple:
    app: str = _tc.get('app').replace('${build}', cwd)
    params: dict = _tc.get('params', {'tracer': [], 'mie': []})
    args: list = _tc.get('args', [])
    _tname = _tc.get('name', '%s%s' % (basename(app), ('-%s' % ('-'.join(args))) if args else ''))
    params_launcher = params.get('launcher', [])
    params_tracer = params.get('tracer', [])
    params_frontend = params.get('frontend', [])
    params_mie = params.get('mie', [])
    _cmd: list = [_runner] + params_launcher
    if params_tracer:
        if _libtracer:
            _cmd += (['-c', _libtracer] + params_tracer)
        else:
            _cmd += params_tracer
    if params_frontend and not _libmie and not _libtracer:
        # these parameters are only used for frontend
        _cmd += params_frontend
    if _libmie:
        _cmd += (['-c', _libmie] + params_mie + ['--', app] + args)
    else:
        _cmd += (params_mie + ['--', app] + args)
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
        while cur < ln:
            line = output[cur].strip()
            if not line:
                cur += 1
                continue
            if checking[-1]:
                _res = pattern in line or re.compile(pattern).match(line)
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
# python 3 runner.py <cwd> <JSON test spec> <emulator-binary> <suite-name> [<one-test>]
#  - cwd: current working directory (it replaces the ${build} placeholder
#              in the JSON test spec)
#  - JSON test spec: description of tests and how to check their results
#  - emulator binary: which emulator executable to use to run the tests
#              (`launcher` means use the launcher from the build tree)
#              (`frontend` means use the `morelloie` frontend from the build tree)
#              (otherwise this must be path to the `morelloie` executable)
#  - suite name: a name of the test suite for the JUnit XML test report
#  - one-test: (optional) can be used to run one specific test from the
#              test suite

if __name__ == '__main__':

    # folder -- current working directory
    # script -- path to the JSON spec file
    # how -- which runner to use (path to the `morelloie` executable)
    # suite_name -- name of the test suite for the JUnit XML report
    folder, script, how, suite_name = sys.argv[1:5]

    # whether this test run is for debug build
    is_debug = 'debug' in suite_name

    # to allow running specific test
    if len(sys.argv) >= 6:
        the_test = sys.argv[5]
        if the_test == '@':
            # run all tests
            the_test = None
        if len(sys.argv) >= 7:
            libc = sys.argv[6]
        else:
            libc = None
    else:
        the_test = None
        libc = None
    if how == 'launcher':
        # use launcher and client libraries from the build tree
        runner = os.path.join(folder, 'launcher/launcher')
        libtracer = os.path.join(folder, 'libtracer/libtracer.so')
        libmie = os.path.join(folder, 'libmie/libmie.so')
    elif how == 'frontend':
        # use `morelloie` from the build tree
        runner = os.path.join(folder, 'launcher/morelloie')
        libtracer = None
        libmie = None
    else:
        # use specific path to the `morelloie` binary
        runner = how
        libtracer = None
        libmie = None

    with open(script, 'rt') as f:
        suite: list = json.load(f)

    testcases = []  # list of results
    ntests, failures, skipped = 0, 0, 0  # counters for tests
    total = 0.0  # total execution time
    skip_reasons = {}  # why tests are skipped

    for tc in suite:

        tname, cmd, xrc, xout, xerr, env = build_test(tc, folder, runner, libtracer, libmie)

        if the_test and tname != the_test:
            # to allow running specific test
            continue

        st = time.time()
        rc, out, err = run_process(cmd, tc.get('stdin', None), int(tc.get('timeout', 5 * 60)), env)
        delta = (time.time() - st)
        total += delta
        ntests += 1
        res, msg = check_results(rc, out.split('\n'), err.split('\n'), xrc, xout, xerr)
        time_str = '%.3f' % delta

        if res:
            print('PASSED  %s: %s (%s sec)' % (tname, msg, time_str))
            testcases.append(passcase.substitute(
                name=tname, suite=suite_name.split('-')[0], time=time_str,
                stdout='' if out is None else out,
                stderr='' if err is None else err))
        else:
            skip = tc.get('skip', 'never')
            if skip != 'never':
                if (is_debug and skip == 'debug')\
                        or (skip == 'musl' and libc == 'musl') \
                        or (skip == 'glibc' and libc == 'glibc') \
                        or (skip == 'jenkins' and 'JOB_URL' in os.environ) \
                        or skip == 'always':
                    skipped += 1
                    print('SKIPPED (%s: %s): %s (%s sec)' % (tname, skip, msg, time_str))
                    testcases.append(skipcase.substitute(
                        name=tname, suite=suite_name.split('-')[0], time=time_str,
                        stdout='' if out is None else out,
                        stderr='' if err is None else err))
                    skipping = True
                    if skip not in skip_reasons:
                        skip_reasons[skip] = 0
                    skip_reasons[skip] += 1
                else:
                    skipping = False
            else:
                skipping = False
            if not skipping:
                failures += 1
                print('FAILED  %s: %s (%s sec)' % (tname, msg, time_str))
                testcases.append(failcase.substitute(
                    name=tname, suite=suite_name.split('-')[0], time=time_str,
                    stdout='' if out is None else out,
                    stderr='' if err is None else err))

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
    if failures > 0:
        print('%s of %s tests failed (%s skipped: %s)' % (failures, ntests - skipped, skipped, ','.join(['%s=%s' % (k, v) for k, v in skip_reasons.items()])))
    else:
        print('All %s tests passed (%s skipped: %s)' % (ntests - skipped, skipped, ','.join(['%s=%s' % (k, v) for k, v in skip_reasons.items()])))
