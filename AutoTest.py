import argparse
import os
import re
import subprocess
import json
import time
from argparse import Namespace
from shutil import which

from TestUtil.Colors import Colors
from TestUtil.ProgressPrinter import ProgressPrinter
from TestUtil.Shell import Shell


class FolderInfo:
    def __init__(self, path):
        info = next(os.walk(path))
        self.name = info[0]
        self.dirs = info[1]
        self.files = info[2]


class RepoScanner:
    def __init__(self):
        p = ProgressPrinter('Scanning repository')
        f = FolderInfo('.')

        self.modules = {'c': {}, 'python': {}}
        for d in f.dirs:
            d_info = FolderInfo(d)
            if 'bootstrap.sh' in d_info.files:
                self.modules['c'][d_info.name] = {}
                self.modules['c'][d_info.name]['tests'] = 'tests' in d_info.dirs
                self.modules['c'][d_info.name]['test-config'] = 'auto-test.json' in d_info.files

        p.succeed()

    def got_module(self, m: str) -> bool:
        return m in self.modules['c'].keys() or m in self.modules[
            'python'].keys()

    def is_c_module(self, m: str) -> bool:
        if not self.got_module(m):
            raise RuntimeError('no such module')

        return m in self.modules['c'].keys()

    def is_python_module(self, m: str) -> bool:
        if not self.got_module(m):
            raise RuntimeError('no such module')

        return m in self.modules['python'].keys()

    def got_tests(self, m: str) -> bool:
        if not self.got_module(m):
            raise RuntimeError('no such module')

        if self.is_c_module(m):
            return self.modules['c'][m]['tests']

        if self.is_python_module(m):
            return self.modules['python'][m]['tests']

    def got_test_config(self, m: str) -> bool:
        if not self.got_module(m):
            raise RuntimeError('no such module')

        if self.is_c_module(m):
            return self.modules['c'][m]['test-config']

        if self.is_python_module(m):
            return self.modules['python'][m]['test-config']


class AutoTest(Namespace):
    def __init__(self):
        super().__init__()
        self.__repo = RepoScanner()
        self.m = None
        self.n = None

    @staticmethod
    def __get_expected_executable__(m: str) -> str:
        # return m.replace('-', '_')
        return 'siot-{}'.format(m)

    def __handle_compile__(self, m: str):
        if not self.__repo.got_module(m):
            Colors.print_warning('no such module: {}'.format(m))
            return

        print('{}{}{}{}'.format(Colors.PINK, Colors.BOLD, m, Colors.RESET))

        if self.__repo.is_c_module(m):
            p = ProgressPrinter('looking for executable bootstrap.sh')
            if not os.access('./{}/bootstrap.sh'.format(m), os.X_OK):
                p.failed()
                return
            else:
                p.succeed()

            p = ProgressPrinter('compiling module {}'.format(m))
            x = Shell(m)
            x.execute_array(['./bootstrap.sh', './configure', 'make'])
            p.finish(x.code)
            if not x.succeed():
                Colors.print_error_output(x.err)
                return

            p = ProgressPrinter('looking for executable "{}"'.format(
                self.__get_expected_executable__(m)))
            if os.access(
                    './{}/{}'.format(m, self.__get_expected_executable__(m)),
                    os.X_OK):
                p.succeed()
            else:
                p.failed()

    @staticmethod
    def __get_test_pairs__(files):
        pairs = []
        file_re = re.compile(r'^(.*)\.(.*)$')

        for f in files:
            match = file_re.match(f)

            if match:
                (name, ext) = match.groups()

                if ext == 'out' and name + '.csv' in files:
                    pairs.append(name)

        return pairs

    def __handle_tests__(self, m: str):
        if not self.__repo.got_module(m):
            Colors.print_warning('no such module: {}'.format(m))
            return

        print('{}{}{}{}'.format(Colors.PINK, Colors.BOLD, m, Colors.RESET))

        if not self.__repo.got_tests(m):
            Colors.print_warning('tests directory not found')
            return

        pre_shell = []
        post_shell = []
        run_args = []

        if self.__repo.got_test_config(m):
            with open('{}/auto-test.json'.format(m)) as json_file:
                data = json.load(json_file)

            pre_shell = data['pre-shell']
            post_shell = data['post-shell']
            run_args = data['run-args']

        if self.__repo.is_c_module(m):
            if which('./{}/{}'.format(m, self.__get_expected_executable__(m))) is None:
                Colors.print_warning('executable file "{}" not found in the "{}" directory'.format(self.__get_expected_executable__(m), m))
                return

            files = FolderInfo('./{}/tests/'.format(m))
            tests = self.__get_test_pairs__(files.files)

            for i in tests:
                p = ProgressPrinter('test {}: {}'.format(tests.index(i), i))

                x = Shell(directory=m)
                x.execute_array(pre_shell)

                a = subprocess.Popen(
                    ['./{}'.format(self.__get_expected_executable__(m)),
                     '-i', 'u:test-in,u:test-out'] + run_args,
                    cwd=m, stderr=subprocess.PIPE)

                time.sleep(1)

                if a.poll() is not None:
                    p.failed()
                    Colors.print_error_output('module crashed after its launch, its last words:')
                    out, err = a.communicate()
                    Colors.print_error_output(err.decode())
                    x = Shell(directory=m)
                    x.execute_array(post_shell)
                    return

                b = subprocess.Popen(['logger', '-i', 'u:test-out', '-w',
                                      './{}/tests/{}.realout'.format(m, i)])

                c = subprocess.Popen(['logreplay', '-i', 'u:test-in', '-f',
                                      './{}/tests/{}.csv'.format(m, i)])

                try:
                    c.communicate(timeout=10)
                except subprocess.TimeoutExpired:
                    if a.poll() is not None:
                        p.failed()
                        Colors.print_error_output('module crashed after its logreplay, its last words:')
                        out, err = a.communicate()
                        Colors.print_error_output(err.decode())
                    else:
                        Colors.print_error_output('Logreplay timeout passed, yet the module did not crash. This should not happen!')

                    a.kill()
                    b.kill()
                    c.kill()
                    x = Shell(directory=m)
                    x.execute_array(post_shell)
                    continue

                time.sleep(3)

                if a.poll() is not None:
                    p.failed()
                    Colors.print_error_output('module crashed after injecting data via logreplay, its last words:')
                    out, err = a.communicate()
                    Colors.print_error_output(err.decode())
                    b.kill()
                    x = Shell(directory=m)
                    x.execute_array(post_shell)
                    continue

                a.kill()
                b.kill()

                d = subprocess.Popen(
                    ['diff', '-s', './{}/tests/{}.realout'.format(m, i),
                     './{}/tests/{}.out'.format(m, i)],
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE)

                d.communicate()
                if d.returncode != 0:
                    p.failed()
                    Colors.print_error_output('unexpected module output')
                    Colors.print_error_output('files ./{0}/tests/{1}.out ./{0}/tests/{1}.realout are different'.format(m, i))
                else:
                    p.succeed()

                x = Shell(directory=m)
                x.execute_array(post_shell)

    def run(self):
        modules = self.__repo.modules['c'].keys()

        if self.m is not None:
            modules = self.m

        if self.n is None or 'C' not in self.n:
            Colors.print_section('COMPILING MODULES')

            for module in modules:
                self.__handle_compile__(module.strip('/'))

        if self.n is not None and 'T' in self.n:
            return

        if which('logger') is None:
            Colors.print_warning('logger does not seem to be installed')
            return

        if which('logreplay') is None:
            Colors.print_warning('logreplay does not seem to be installed')
            return

        Colors.print_section('TESTING MODULES')

        for module in modules:
            self.__handle_tests__(module.strip('/'))


def main():
    parser = argparse.ArgumentParser(description='Auto testing for NEMEA-SIoT.')
    parser.add_argument('-m', metavar='M', type=str, nargs='+',
                        help='specify module to be tested')

    parser.add_argument('-n', choices='TC', action='append',
                        help='do not [C]ompile / [T]est')

    auto_test = AutoTest()
    parser.parse_args(namespace=auto_test)

    auto_test.run()


if __name__ == '__main__':
    main()
