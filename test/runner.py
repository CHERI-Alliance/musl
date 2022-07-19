import sys
import json
import time
import os
import subprocess
import re

from os.path import basename
from multiprocessing import Pool
from string import Template
from datetime import datetime

report = Template('''<?xml version="1.0" encoding="UTF-8"?>
<testsuites>
<testsuite name="${suite}" errors="${errors}" failures="${failures}" skipped="${skipped}" tests="${tests}" hostname="tbd" time="${total}" timestamp="${timestamp}">
${testcases}
</testsuite>
</testsuites>
''')
passcase = Template('''<testcase classname="${classname}" name="${name}" time="${time}" status="run">
<system-out><![CDATA[
${stdout}
]]></system-out>
<system-err><![CDATA[
${stderr}
]]></system-err>
</testcase>''')
failcase = Template('''<testcase classname="${classname}" name="${name}" time="${time}" status="run">
<failure message="Test ${name} failed" type="failure">
<![CDATA[${description}]]>
</failure>
<system-out><![CDATA[
${stdout}
]]></system-out>
<system-err><![CDATA[
${stderr}
]]></system-err>
</testcase>''')
skipcase = Template('''<testcase classname="${classname}" name="${name}" time="${time}" status="run">
<skipped/>
<system-out><![CDATA[
${stdout}
]]></system-out>
<system-err><![CDATA[
${stderr}
]]></system-err>
</testcase>''')

class TC:
    PASS = '\033[1;32m' if sys.stdout.isatty() else ''
    FAIL = '\033[1;31m' if sys.stdout.isatty() else ''
    WARN = '\033[1;33m' if sys.stdout.isatty() else ''
    CEND = '\033[0m' if sys.stdout.isatty() else ''

class Args(object):

    def __init__(self):
        opt = sys.argv[1:]
        self.kind: str = opt[0]  # kind of test run (e.g. debug or release)
        self.nproc: int = int(opt[1])  # number of parallel processes
        self.workdir: str = opt[2]  # working directory
        self.testspec: str = opt[3]  # path to the JSON test spec
        self.morelloie: str = opt[4]  # path to the Morello IE executable
        self.suitename: str = opt[5]  # testsuite name (classname for junit report)
        self.report: str = opt[6]  # path to junit report
        self.classname: str = f'{self.suitename}.{self.kind}'

class Test(object):

    def __init__(self, t: dict, a: Args):
        self.app: str = Test.replace_variables(a, t.get('app'))   # test app
        self.args: list = [Test.replace_variables(a, v) for v in t.get('args', [])]  # arguments for test app
        self.params: list = [Test.replace_variables(a, v) for v in t.get('params', [])]  # Morello IE parameters for this test
        self.name: str = t.get('name', self.default_name())  # test name
        self.timeout: int = int(t.get('timeout', 120))  # test timeout in seconds
        self.env: dict = {k: v for k, v in t.get('env', {}).items()}  # extra env vars
        self.skip: list = Test.skip_reasons(t)  # list of reasons to ignore failure
        self.file: dict = Test.temp_file(t)
        self.stdin: str = t.get('stdin', None)  # standard input (optional)
        self.expected: dict = {  # expected results from the test run
            'rc': Test.expected_rc(t),  # exit code
            'stdout': t.get('stdout', []),  # lines to match in stdout
            'stderr': t.get('stderr', []),  # lines to match in stderr
        }
        self.kind: str = a.kind
        self.mie: bool = t.get('mie', a.morelloie != 'native')  # whether to use Morello IE
        if self.mie:
            if 'TEST_RUNNER_MIEARGS' in os.environ:
                extra_args = os.environ['TEST_RUNNER_MIEARGS'].strip()
                if extra_args:
                    extra = [v.strip() for v in extra_args.split(' ')]
                else:
                    extra = []
            else:
                extra = []
            self.cmd: list = [a.morelloie] + extra + self.params + ['--', self.app] + self.args
        else:
            self.cmd: list = [self.app] + self.args
        self.cleanup: list = [Test.replace_variables(a, v) for v in t.get('cleanup', [])]  # clean up command

    @staticmethod
    def replace_variables(a: Args, text: str) -> str:
        tmp = text.replace('${build}', a.workdir)
        tmp = tmp.replace('${kind}', a.kind)
        for name, val in os.environ.items():
            if name.startswith('TEST_RUNNER_'):
                nm = name[len('TEST_RUNNER_'):].lower()
                tmp = tmp.replace(f'${{{nm}}}', val)
        return tmp

    def default_name(self):
        base = basename(self.app).replace('.', '-').replace('_', '-')
        suffix = '-' + str('-'.join([str(t).strip(' -') for t in self.args])) if self.args else ''
        postfix = '-' + str('-'.join([str(t).strip(' -') for t in self.params])) if self.params else ''
        return f'{base}{suffix}{postfix}'

    @staticmethod
    def skip_reasons(t: dict) -> list:
        skip = t.get('skip', [])
        if not isinstance(skip, list):
            skip = [str(skip)]
        return skip

    @staticmethod
    def temp_file(t: dict) -> dict:
        file = t.get('file', {})
        if file:
            return {
                'path': file['path'],  # where to create file
                'contents': file.get('contents', ''),  # text to put in the file
                'keep': file.get('keep', False)  # keep the file after tests run?
            }
        else:
            return {}  # means no file needed

    @staticmethod
    def expected_rc(t: dict) -> list:
        xrc = t.get('xrc', [0])
        if not isinstance(xrc, list):
            xrc = [xrc]
        return [int(v) for v in xrc]

def __subprocess(cmd: list, stdin, env: dict, timeout: int):
    result = subprocess.run(cmd, universal_newlines=True,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE, input=stdin,
        timeout=timeout, env={**env, **dict(os.environ)})
    return result

def __run_process(t: Test) -> tuple:
    try:
        result = __subprocess(t.cmd, t.stdin, t.env, t.timeout)
        stdout = result.stdout
        stderr = result.stderr
        code = result.returncode
    except subprocess.TimeoutExpired as err:
        stdout = err.stdout
        stderr = err.stderr
        if not stdout:
            stdout = f'Timed out after {t.timeout} seconds'
        if not stderr:
            stderr = f'Timed out after {t.timeout} seconds'
        code = 217
    return code, stdout.strip('\n'), stderr.strip('\n')

def __scan_output(t: Test, received: list, expected: list) -> tuple:
    n = 0
    find_next = False
    tnm = t.name
    for kk, cmd in enumerate(expected):
        if isinstance(cmd, dict):
            query = cmd.get('command', '<none>')
            iff = cmd.get('if', [])
            if iff and t.kind not in iff:  # skip command if it's not applicable
                continue
            if query == 'find-next-match':
                if find_next:
                    yield '', '', -4  # bad command
                find_next = True
                continue
            elif query == 'ignore-the-rest':
                yield '', '', -1  # end of validation commands
            else:
                print(f'{TC.FAIL}{tnm}: bad command {query} in line {kk} in expected output{TC.CEND}', file=sys.stderr)
                sys.exit(2)
        if find_next:
            found = False
            line = None
            while not found:
                if n >= len(received):
                    # print(f'{TC.WARN}{tnm}: reached end of output at line {n - 1}{TC.CEND}', file=sys.stderr)
                    yield '', '', -2  # reached end of output
                line = received[n]
                n += 1
                if cmd in line:
                    # check that `expected` is just a substring
                    found = True
                else:
                    # try to match regex
                    matcher = re.compile(cmd)
                    if matcher.match(line):
                        found = True
            find_next = False
            yield line, cmd, n  # we index lines starting with 1
        else:
            if n >= len(received):
                # print(f'{TC.WARN}{tnm}: reached end of output at line {n - 1}{TC.CEND}', file=sys.stderr)
                yield '', '', -2  # reached end of output
            line = received[n]
            n += 1
            yield line, cmd, n  # we index lines starting with 1
    if n < len(received):
        # print(f'{TC.WARN}{tnm}: reached end of expected output{TC.CEND}', file=sys.stderr)
        yield '', '', -3  # reached end expected output but more lines are available

def __compare_output(t: Test, received: list, expected: list) -> tuple:
    if not expected:
        return True, ''  # no need to check anything
    for v in __scan_output(t, received, expected):
        line, paragon, n = v
        if n == -1:
            # no more checks needed
            break
        elif n == -2:
            # unexpected end of output
            return False, f'(reached end of output)'
        elif n == -3:
            # end of expected output
            return False, f'(output is too long)'
        elif n == -4:
            # bad command in test spec
            return False, f'(bad command in test spec)'
        if paragon in line:
            # check that `expected` is just a substring
            pass
        else:
            # try to match regex
            matcher = re.compile(paragon)
            if matcher.match(line):
                pass
            else:
                return False, f'in line {n} `{line}` vs `{paragon}`'
    return True, ''  # all good

def __check(rc: int, stdout: list, stderr: list, t: Test) -> tuple:
    message = []
    successful = rc in t.expected.get('rc')
    if not successful:
        message += [f'unexpected exit code: {rc}']
    successful, where = __compare_output(t, stdout, t.expected.get('stdout'))
    if not successful:
        message += [f'stdout mismatch {where}']
    successful, where = __compare_output(t, stderr, t.expected.get('stderr'))
    if not successful:
        message += [f'stderr mismatch {where}']
    return len(message) == 0, ', '.join(message)

def __run(t: Test, a: Args) -> tuple:
    if t.file:
        # create tmp text file before running the test
        with open(t.file.get('path'), 'wt') as tf:
            tf.write(t.file.get('contents', ''))
    st = time.time()
    rc, out, err = __run_process(t)
    dt = (time.time() - st)
    dt_str = '%.3f' % dt
    if t.file and not t.file.get('keep'):
        # remove tmp text file after running the test
        if os.path.exists(t.file.get('path')):
            os.remove(t.file.get('path'))
    if t.cleanup:
        __subprocess(t.cleanup, None, {}, None)
    successful, msg = __check(rc, out.split('\n'), err.split('\n'), t)
    nskipped = 0
    nfailed = 0
    skipping = ''
    if successful:
        print(f'{TC.PASS}PASS{TC.CEND} {t.name} ({dt_str} sec)')
        res = passcase.substitute(
            name=t.name, classname=a.classname, time=dt_str,
            stdout='' if out is None else out,
            stderr='' if err is None else err)
    else:
        if t.skip:
            if a.kind in t.skip:
                skipping = a.kind
            elif 'jenkins' in t.skip and 'JOB_URL' in os.environ:
                skipping = 'jenkins'
            elif 'flaky' in t.skip or 'always' in t.skip:
                skipping = 'flaky'
        if skipping:
            nskipped = 1
            print(f'{TC.WARN}SKIP{TC.CEND} {t.name}: {msg} [{skipping}] ({dt_str} sec)')
            res = skipcase.substitute(
                name=t.name, classname=a.classname, time=dt_str,
                stdout='' if out is None else out,
                stderr='' if err is None else err)
        else:
            nfailed = 1
            print(f'{TC.FAIL}FAIL{TC.CEND} {t.name}: {msg} ({dt_str} sec)')
            res = failcase.substitute(
                name=t.name, classname=a.classname, time=dt_str,
                description=f'Command line: {" ".join(t.cmd)}\nReason: {msg}\nReturn code: {rc}',
                stdout='' if out is None else out,
                stderr='' if err is None else err)
    return t.name, res, dt, nskipped, nfailed, skipping

if __name__ == '__main__':
    args = Args()
    with open(args.testspec, 'rt') as f:
        testspec: list = json.load(f)
    tests = [Test(v, args) for v in testspec]
    testnames = set([v.name for v in tests])
    if len(testnames) < len(tests):
        print(f'Duplicated test names found', file=sys.stderr)
        sys.exit(3)
    if args.nproc == 1:
        q: list = [__run(t, args) for t in tests]
    else:
        def __run__(v: Test):
            return __run(v, args)
        with Pool(args.nproc) as p:
            q: list = p.map(__run__, tests)
    testcases = []  # list of results for junit report
    ntests, failures, skipped = 0, 0, 0  # counters for tests
    failed_tests = []  # list of failed tests
    skipped_tests = []  # list of skipped tests
    total = 0.0  # total execution time
    skip_reasons = {}  # why tests have been skipped
    for x in q:
        if x is None:
            continue
        tname, tres, ttime, tskipped, tfailed, tsreason = x
        if tres is None:
            continue
        testcases.append(tres)
        total += ttime
        ntests += 1
        skipped += tskipped
        failures += tfailed
        if tfailed > 0:
            failed_tests.append(tname)
        if tskipped > 0:
            skipped_tests.append((tname, tsreason))
            if tsreason not in skip_reasons:
                skip_reasons[tsreason] = 0
            skip_reasons[tsreason] += tskipped
    when = datetime.now()
    with open(args.report, 'wt') as f:
        f.write(report.substitute(
            testcases='\n'.join(testcases),
            suite=args.suitename,
            errors='0',
            failures=f'{failures}', skipped=f'{skipped}', tests=f'{ntests}',
            timestamp=when.astimezone().strftime('%Y-%m-%dT%H:%M:%S%z'),
            total='%.3f' % total
        ))
    print(f'Results: total tests {ntests}, failed {failures}, skipped {skipped}')
    if failures > 0:
        print('Failed tests:')
    for tn in failed_tests:
        print(f'- failed: {tn}')
    if skipped > 0:
        print('Skipped tests:')
    for tn, tsreason in skipped_tests:
        print(f'- skipped: {tn} ({tsreason})')
    sys.exit(1 if failures > 0 else 0)
