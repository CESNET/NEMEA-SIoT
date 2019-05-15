import argparse
import json
import os
import re
import subprocess
import time
from argparse import Namespace
from shutil import which, copy2
from pathlib import Path

from TestUtil.Colors import Colors
from TestUtil.ProgressPrinter import ProgressPrinter
from TestUtil.Shell import Shell
from TestUtil.RepoScanner import RepoScanner


class IntegrationTest(Namespace):
    def __init__(self):
        super().__init__()
        self.__t = True
        self.__repo = RepoScanner('.')
        self.__t_meta = {
            'config': Path("/etc/config/nemea-supervisor"),
            'test-config': Path("./TestUtil/turris-supervisorl-test.conf")
        }

        self.__t_meta['config-backup'] = Path('{}.backup'.format(self.__t_meta['config'].name))

    def __t_backup_sv_conf__(self):
        sv_conf = self.__t_meta['config']
        if sv_conf.is_file():
            sv_conf_backup = Path('{}/{}.backup'.format(os.getcwd(), sv_conf.name))
            copy2(sv_conf, sv_conf_backup)
            self.__t_meta['config-backup'] = sv_conf_backup

    def __t_restore_sv_conf__(self):
        if self.__t_meta['config-backup'].is_file():
            self.__t_cp_to_sv_conf_loc__(self.__t_meta['config-backup'])
            os.remove(self.__t_meta['config-backup'])

    def __t_cp_to_sv_conf_loc__(self, path: Path):
        copy2(path, self.__t_meta['config'])

    def __handle_tests__(self, m: str, tp: []):
        if not self.__repo.got_module(m):
            Colors.print_warning('no such module: {}'.format(m))
            return

        print('{}{}{}{}'.format(Colors.PINK, Colors.BOLD, m, Colors.RESET))

        if not self.__repo.got_tests(m):
            Colors.print_warning('tests directory not found')
            return

            x = Shell()
            x.execute('mkdir i-tests-outs/{}'.format(m))

        if self.__repo.is_c_module(m):
            for i in tp:
                p = ProgressPrinter('test {}: {}'.format(tp.index(i) + 1, i))

                b = subprocess.Popen(
                    ['/usr/bin/nemea/logger', '-i', 'u:tout-{}'.format(m), '-w',
                     './i-tests-outs/{}/{}.realout'.format(m, i)])

                c = subprocess.Popen(['/usr/bin/nemea/logreplay', '-i', 'u:tin-{}'.format(m), '-f',
                                      './{}/tests/{}.csv'.format(m, i)])

                time.sleep(10)
                b.kill()

                d = subprocess.Popen(
                    ['diff', '-i', '-s', './i-tests-outs/{}/{}.realout'.format(m, i),
                     './{}/tests/{}.out'.format(m, i)],
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE)

                d.communicate()
                if d.returncode != 0:
                    p.failed()
                    Colors.print_error_output('unexpected module output')
                    Colors.print_error_output(
                        'files ./{0}/tests/{1}.out ./{0}/tests/{1}.realout are different'.format(
                            m, i))
                else:
                    p.succeed()

    def __test_on_t__(self):
        supervisor = Path("/etc/init.d/nemea-supervisorl")
        if not supervisor.is_file():
            pass
            # return

        subprocess.Popen([supervisor, 'stop'])
        self.__t_backup_sv_conf__()
        self.__t_cp_to_sv_conf_loc__(self.__t_meta['test-config'])
        subprocess.Popen([supervisor, 'reload'])
        subprocess.Popen([supervisor, 'start'])

        modules = self.__repo.modules['c'].keys()
        input("Press Enter to continue...")

        x = Shell()
        x.execute('mkdir i-tests-outs')

        for m in modules:
            if self.__repo.got_tests(m):
                self.__handle_tests__(m, self.__repo.get_tests(m))

        input("Press Enter to continue...")

        subprocess.Popen([supervisor, 'stop'])
        self.__t_restore_sv_conf__()

    def run(self):
        if self.__t:
            if self.r:
                self.__t_restore_sv_conf__()
                return

            self.__test_on_t__()



def main():
    parser = argparse.ArgumentParser(description='Auto testing for NEMEA-SIoT.')
    parser.add_argument('-m', metavar='M', type=str, nargs='+',
                        help='specify module to be tested')

    parser.add_argument('-n', choices='TC', action='append',
                        help='do not [C]ompile / [T]est')

    parser.add_argument('-r', action='store_true',
                        help='Just restore nemea-supervisorl config back-up')

    auto_test = IntegrationTest()
    parser.parse_args(namespace=auto_test)

    auto_test.run()


if __name__ == '__main__':
    main()
