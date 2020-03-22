import subprocess
import signal
from shutil import which
from .RepoScanner import RepoScanner
from .Shell import Shell
from argparse import Namespace, ArgumentParser, ArgumentError

class SIoTNemeaTester(Namespace):
    @staticmethod
    def __get_tests_output_dir() -> str:
        return 'tests-out'

    @staticmethod
    def __get_module_tests_dir() -> str:
        return 'tests'

    @staticmethod
    def __get_test_in_paths__(m: str, t: (str, {str, })) -> [str]:
        name, ifcs = t
        paths = []
        for i in ifcs['in']:
            path = './{}/{}/{}.{}.csv'.format(m, SIoTNemeaTester.__get_module_tests_dir(), name, i)
            paths.append(path)
        return paths

    @staticmethod
    def __get_test_expected_out_paths__(m: str, t: (str, {str, })) -> [str]:
        name, ifcs = t
        paths = []
        for i in ifcs['out']:
            path = './{}/{}/{}.{}.out'.format(m, SIoTNemeaTester.__get_module_tests_dir(), name, i)
            paths.append(path)
        return paths

    @staticmethod
    def __get_test_out_paths__(m: str, t: (str, {str, })) -> [str]:
        name, ifcs = t
        paths = []
        for i in ifcs['out']:
            path = './{}/{}/{}.{}.realout'.format(SIoTNemeaTester.__get_tests_output_dir(), m, name, i)
            paths.append(path)
        return paths

    @staticmethod
    def __prepare_test_output_directories__(modules: []):
        x = Shell()
        x.execute('rm -rf {0} && mkdir {0}'.format(SIoTNemeaTester.__get_tests_output_dir()))
        y = Shell(SIoTNemeaTester.__get_tests_output_dir())
        y.execute('mkdir {}'.format(' '.join(modules)))

    def __signal_exit__(self, signum, frame):
        print("\n[!] SIoTNemeaTester: signal caught - shutting down.")
        self.quit = True
        self.d = False
        self.__clean_action__()

    def __clean_action__(self):
        pass

    def __init__(self, arg_parser: ArgumentParser):
        signal.signal(signal.SIGINT, self.__signal_exit__)
        self.quit = False

        arg_parser.add_argument('-m', metavar='M', type=str, nargs='+', help='specify module to be tested')
        self.m = []

        arg_parser.add_argument('-L', metavar='path', type=str, help='path to the NEMEA Logger, default: /usr/bin/nemea/logger')
        self.L = '/usr/bin/nemea/logger'

        arg_parser.add_argument('-R', metavar='path', type=str, help='path to the NEMEA Logreplay, default: /usr/bin/nemea/logreplay')
        self.R = '/usr/bin/nemea/logreplay'

        arg_parser.add_argument('-d', action='store_true', help='print test details')
        self.d = False

        arg_parser.parse_args(namespace=self)
        self.parser = arg_parser

        if which(self.L) is None:
            raise ArgumentError('missing logger: {} is not an executable file'.format(self.L))

        if which(self.R) is None:
            raise ArgumentError('missing logreplay: {} is not an executable file'.format(self.R))

        if which('diff') is None:
            raise ArgumentError('missing diff utility')

        self.m = list(map(lambda m: m.strip('/'), self.m)) # strip trailing / from modules to be tested
        self.repo = RepoScanner()
        self.repo.filter(self.m)

        self.results = {}
        self.module_being_tested = None
        self.current_test = None

    def __inject_and_capture__(self, ifcs_in: [str], files_in: [str], ifcs_out: [str], files_out: [str], timeout: int = 10):
        if len(ifcs_in) != len(files_in):
            raise RuntimeError('logreplays: count of input interfaces differ from count of input files')

        if len(ifcs_out) != len(files_out):
            raise RuntimeError('loggers: count of output interfaces differ from count of output files')

        loggers = []
        zipped_output = zip(ifcs_out, files_out)
        for ifc, file in zipped_output:
            logger = subprocess.Popen([self.L, '-i', 'u:out{}'.format(ifc), '-w', file])
            loggers.append(logger)

        logreplays = []
        zipped_input = zip(ifcs_in, files_in)
        for ifc, file in zipped_input:
            logreplay = subprocess.Popen([self.R, '-i', 'u:in{}'.format(ifc), '-f', file])
            logreplays.append(logreplay)

        killed = False
        for logreplay in logreplays:
            if killed:
                logreplay.kill()

            try:
                logreplay.communicate(timeout=timeout)
            except subprocess.TimeoutExpired:
                logreplay.kill()
                killed = True

        for logger in loggers:
            if killed:
                logger.kill()

            try:
                logger.communicate(timeout=timeout)
            except subprocess.TimeoutExpired:
                logger.kill()
                killed = True

        if killed:
            raise RuntimeError('logreplay/logger timeout expired')


    def __compare_files__(self, expected_files: [str], to_compare_files: [str]) -> bool:
        if len(expected_files) != len(to_compare_files):
            raise RuntimeError('count of expected files differ from count of output files')

        zipped = zip(expected_files, to_compare_files)
        for expected, to_compare in zipped:
            diff = subprocess.Popen(['diff', '-s', expected, to_compare], stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE)
            diff.communicate()

            if diff.returncode != 0:
                return False

        return True

    def __test_started__(self, m: str, t: (str, {str, })):
        if not m == self.module_being_tested:
            self.module_being_tested = m
            self.__new_module_tests_started__()

        self.current_test, _ = t
        self.__new_test_started__()

    def __test_ended__(self, r: bool):
        if self.module_being_tested not in self.results:
            self.results[self.module_being_tested] = {}

        self.results[self.module_being_tested][self.current_test] = r

        if self.d:
            print('\r [{}] {}'.format(('fail',' ok ')[r], self.current_test))


    def __new_module_tests_started__(self):
        if self.d:
            print('testing: {}'.format(self.module_being_tested))

    def __new_test_started__(self):
        if self.d:
            print(' [ -- ] {}'.format(self.current_test), end='', flush=True)

    def __finished__(self) -> int:
        ok = 0
        failed = 0

        for m in self.results:
            for t in self.results[m]:
                if self.results[m][t]:
                    ok += 1
                else:
                    failed += 1

        if failed == 0:
            print('ok({})'.format(ok))
            return 0
        else:
            print('not ok - {} failed, {} ok'.format(failed, ok))
            return 1
