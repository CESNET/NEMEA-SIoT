# This file was created within Martin Hošala's bachelor thesis
#
# THESIS INFORMATION:
#
# School: Brno University of Technology
# Faculty: Faculty of Information Technology
# Department: Department of Computer Systems
# Topic: Secured Gateway for Wireless IoT Protocols
# Author: Martin Hošala
# Supervisor: Doc. Ing. Jan Kořenek Ph.D.
# Year: 2019

import argparse
import os
import signal
import subprocess
import time
from argparse import Namespace
from shutil import copy2, which
from pathlib import Path

from TestUtil.Colors import Colors
from TestUtil.ProgressPrinter import ProgressPrinter
from TestUtil.Shell import Shell
from TestUtil.RepoScanner import RepoScanner


# This class serves for NEMEA-SIoT modules integration testing. Its detailed
# description can be found in the REAMDE.md file present in NEMEA-SIoT
# repository and bachelor's thesis text. This script is  suitable only for
# testing on the Turris Omnia router at the moment. Also, it must be launched
# from a root directory of NEMEA SIoT repository, otherwise its behavior
# is not defined.
class IntegrationTest(Namespace):
    def signal_exit(self, signum, frame):
        print("IntegrationTest: signal caught - shutting down.")
        self.__t_restore_sv_conf__()

        if self.__p is not None:
            self.__p.stop()

        self.__quit = True
        return

    def __init__(self):
        super().__init__()

        signal.signal(signal.SIGINT, self.signal_exit)
        self.__quit = False

        self.__t = True  # testing on Turris Omnia - at the moment always true
        self.__p = None

        self.p = False
        self.R = None
        self.L = None
        self.__repo = RepoScanner('.')
        self.__t_meta = {
            'config': Path("/etc/config/nemea-supervisor"),
            'test-config': Path("./TestUtil/turris-supervisorl-test.conf")
        }

        self.__t_meta['config-backup'] = Path('{}.backup'.format(self.__t_meta['config'].name))

    # This method backups NEMEA SupervisorL configuration which is already
    # present at the router
    def __t_backup_sv_conf__(self):
        sv_conf = self.__t_meta['config']
        if sv_conf.is_file():
            sv_conf_backup = Path('{}/{}.backup'.format(os.getcwd(), sv_conf.name))
            copy2(sv_conf, sv_conf_backup)
            self.__t_meta['config-backup'] = sv_conf_backup

    # This method serves to restore the original NEMEA SupervisorL configuration
    # from the backup
    def __t_restore_sv_conf__(self):
        if self.__t_meta['config-backup'].is_file():
            self.__t_cp_to_sv_conf_loc__(self.__t_meta['config-backup'])
            os.remove(self.__t_meta['config-backup'])

    # Copies a file to location of the NEMEA SupervisorL configuration file
    def __t_cp_to_sv_conf_loc__(self, path: Path):
        if not path.is_file():
            print('hm')

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
        x.execute('mkdir i-test-out/{}'.format(m))

        if self.__repo.is_c_module(m):
            for i in tp:
                if self.__quit:
                    return

                if m == 'ble-pairing':
                    x = Shell()
                    x.execute('rm -rf /tmp/ble-pairing-test/*')

                self.__p = ProgressPrinter('test {}: {}'.format(tp.index(i) + 1, i))

                output_file_path = 'i-test-out/{}/{}.realout'.format(m, i)
                input_file_path = './{}/tests/{}.csv'.format(m, i)
                expected_file_path = './{}/tests/{}.out'.format(m, i)

                # connects NEMEA Logger to module output
                logger = subprocess.Popen(
                    [self.L, '-i', 'u:tout-{}'.format(m), '-w',
                     output_file_path])

                # injects data to a module input IFC using NEMEA Logreplay
                logreplay = subprocess.Popen([self.R, '-n', '-i', 'u:tin-{}'.format(m), '-f', input_file_path])

                # if the Logreplay is still running after 10 seconds the module
                # probably failed orcurrent test data set is really huge
                try:
                    logreplay.communicate(timeout=10)
                except subprocess.TimeoutExpired:
                    self.__p.failed()
                    Colors.print_error_output(
                            'Dataset that is being replayed now whether really huge or the module crashed.')

                    logger.kill()
                    logreplay.kill()
                    continue

                time.sleep(4)
                logger.kill()

                # compares real module output with the expected one
                diff = subprocess.Popen(
                    ['diff', '-s', '-i', output_file_path, expected_file_path],
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE)

                diff.communicate()
                if diff.returncode != 0:
                    self.__p.failed()
                    Colors.print_error_output('unexpected module output')
                    Colors.print_error_output(
                        'files {} {} differ!'.format(output_file_path,
                                                     expected_file_path))
                else:
                    self.__p.succeed()

    # Main method for testing on the Turris Omnia which provides the side tasks
    # necessary for proper testing
    def __test_on_t__(self):
        supervisor = Path("/etc/init.d/nemea-supervisorl")
        if not supervisor.is_file():
            Colors.print_warning('NEMEA SupervisorL not found: {} is not a file'.format(supervisor))
            return

        # backs up the original configuration for NEMEA SupervisorL and replaces
        # it with the one for testing
        self.__t_backup_sv_conf__()
        self.__t_cp_to_sv_conf_loc__(self.__t_meta['test-config'])
        subprocess.Popen([supervisor, 'reload'])
        subprocess.Popen([supervisor, 'start'])

        modules = self.__repo.modules['c'].keys()

        if self.m is not None:
            modules = [x.strip('/') for x in self.m]

        if self.p and not self.__quit:
            input("You can now verify whether the modules are running - Press Enter to continue...")

        x = Shell()
        x.execute('mkdir i-test-out')

        for m in modules:
            if self.__quit:
                return

            if m == 'ble-pairing':
                x = Shell()
                x.execute('mkdir /tmp/ble-pairing-test/')

            if self.__repo.got_tests(m):
                self.__handle_tests__(m, self.__repo.get_tests(m))

            if m == 'ble-pairing':
                x = Shell()
                x.execute('rm -rf /tmp/ble-pairing-test/')

        if self.p and not self.__quit:
            input("You can now verify whether the modules are still running - Press Enter to continue...")

        subprocess.Popen([supervisor, 'stop'])
        self.__t_restore_sv_conf__()

    def run(self):
        if self.L is not None:
            if which(self.L) is None:
                Colors.print_warning('{} is no an executable file'.format(self.L))
                return

        elif which('/usr/bin/nemea/logger') is None:
            Colors.print_warning('logger does not seem to be installed')
            return

        else:
            self.L = '/usr/bin/nemea/logger'

        if self.R is not None:
            if which(self.R) is None:
                Colors.print_warning('{} is no an executable file'.format(self.R))
                return

        elif which('/usr/bin/nemea/logreplay') is None:
            Colors.print_warning('logreplay does not seem to be installed')
            return
        else:
            self.R = '/usr/bin/nemea/logreplay'

        if self.__t:
            if self.r:
                self.__t_restore_sv_conf__()
                return

            self.__test_on_t__()


def main():
    parser = argparse.ArgumentParser(description='Auto testing for NEMEA-SIoT.')
    parser.add_argument('-m', metavar='M', type=str, nargs='+',
                        help='specify module to be tested')

    parser.add_argument('-r', action='store_true',
                        help='Just restore nemea-supervisorl config back-up')

    parser.add_argument('-p', action='store_true',
                        help='Pauses the script before and after the test so that it is possible to verify wether modules are correctly running')

    parser.add_argument('-L', metavar='path', type=str,
                        help='path to the NEMEA Logger, if not specified Logger is considered to be installed')

    parser.add_argument('-R', metavar='path', type=str,
                        help='path to the NEMEA Logreplay, if not specified Logreplay is considered to be installed')

    integration_test = IntegrationTest()
    parser.parse_args(namespace=integration_test)

    integration_test.run()


if __name__ == '__main__':
    try:
        main()
    except Exception as e:
        print('ERROR: {}'.format(e))
