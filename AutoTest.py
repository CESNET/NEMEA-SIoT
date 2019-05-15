import argparse
import json
import os
import re
import subprocess

from argparse import Namespace
from shutil import which

from TestUtil.Colors import Colors
from TestUtil.FolderInfo import FolderInfo
from TestUtil.ProgressPrinter import ProgressPrinter
from TestUtil.RepoScanner import RepoScanner
from TestUtil.Shell import Shell


class AutoTest(Namespace):
    def __init__(self):
        super().__init__()
        self.__repo = RepoScanner()
        self.m = None
        self.n = None
        self.L = None
        self.R = None

    @staticmethod
    def __get_expected_executable__(m: str) -> str:
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

        x = Shell()
        x.execute('mkdir a-test-out/{}'.format(m))

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
            if which('./{}/{}'.format(m, self.__get_expected_executable__(
                    m))) is None:
                Colors.print_warning(
                    'executable file "{}" not found in the "{}" directory'.format(
                        self.__get_expected_executable__(m), m))
                return

            files = FolderInfo('./{}/tests/'.format(m))
            tests = self.__get_test_pairs__(files.files)

            for i in tests:
                p = ProgressPrinter('test {}: {}'.format(tests.index(i) + 1, i))

                x = Shell(directory=m)
                x.execute_array(pre_shell)

                module = subprocess.Popen(
                    ['./{}'.format(self.__get_expected_executable__(m)),
                     '-i', 'u:test-in,u:test-out'] + run_args,
                    cwd=m, stderr=subprocess.PIPE)

                if module.poll() is not None:
                    p.failed()
                    Colors.print_error_output(
                        'module crashed after its launch, its last words:')
                    out, err = module.communicate()
                    Colors.print_error_output(err.decode())
                    x = Shell(directory=m)
                    x.execute_array(post_shell)
                    return

                output_file_path = 'a-test-out/{}/{}.realout'.format(m, i)
                input_file_path = './{}/tests/{}.csv'.format(m, i)
                expected_file_path = './{}/tests/{}.out'.format(m, i)

                logger = subprocess.Popen(
                    [self.L, '-i', 'u:test-out', '-w',
                     output_file_path])

                logreplay = subprocess.Popen([self.R, '-i', 'u:test-in', '-f',
                                              input_file_path])

                try:
                    logreplay.communicate(timeout=10)
                except subprocess.TimeoutExpired:
                    if module.poll() is not None:
                        p.failed()
                        Colors.print_error_output(
                            'module crashed after its logreplay, its last words:')
                        out, err = module.communicate()
                        Colors.print_error_output(err.decode())
                    else:
                        Colors.print_error_output(
                            'Logreplay timeout passed, yet the module did not crash. This should not happen! Dataset that is being replayed now is probably really huge.')

                    module.kill()
                    logger.kill()
                    logreplay.kill()
                    x = Shell(directory=m)
                    x.execute_array(post_shell)
                    continue

                try:
                    module.communicate(timeout=20)
                except subprocess.TimeoutExpired:
                    Colors.print_error_output('something bad happened')
                    module.kill()
                    logger.kill()
                    x = Shell(directory=m)
                    x.execute_array(post_shell)
                    continue

                logger.kill()

                diff = subprocess.Popen(
                    ['diff', '-s', output_file_path, expected_file_path],
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE)

                diff.communicate()
                if diff.returncode != 0:
                    p.failed()
                    Colors.print_error_output('unexpected module output')
                    Colors.print_error_output(
                        'files {} {} differ!'.format(output_file_path,
                                                     expected_file_path))
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

        if self.L is not None:
            if which(self.L) is None:
                Colors.print_warning('{} is no an executable file'.format(self.L))
                return

        elif which('logger') is None:
            Colors.print_warning('logger does not seem to be installed')
            return

        else:
            self.L = 'logger'

        if self.R is not None:
            if which(self.R) is None:
                Colors.print_warning('{} is no an executable file'.format(self.R))
                return

        elif which('logreplay') is None:
            Colors.print_warning('logreplay does not seem to be installed')
            return
        else:
            self.R = 'logreplay'

        Colors.print_section('TESTING MODULES')

        x = Shell()
        x.execute('mkdir a-test-out')

        for module in modules:
            self.__handle_tests__(module.strip('/'))


def main():
    parser = argparse.ArgumentParser(description='Auto testing for NEMEA-SIoT.')
    parser.add_argument('-m', metavar='M', type=str, nargs='+',
                        help='specify module to be tested')

    parser.add_argument('-n', choices='TC', action='append',
                        help='do not [C]ompile / [T]est')

    parser.add_argument('-L', metavar='path', type=str,
                        help='path to the NEMEA Logger, if not specified Logger is considered to be installed')

    parser.add_argument('-R', metavar='path', type=str,
                        help='path to the NEMEA Logreplay, if not specified Logreplay is considered to be installed')

    auto_test = AutoTest()
    parser.parse_args(namespace=auto_test)

    auto_test.run()


if __name__ == '__main__':
    main()
