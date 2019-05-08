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


class IntegrationTest(Namespace):
    def __init__(self):
        super().__init__()
        self.__t = True
        # self.__t_meta = {'config': Path("/etc/config/nemea-supervisor")}
        self.__t_meta = {'config': Path("./TestUtil/turris-supervisorl-test.conf")}

    def __t_backup_sv_conf__(self):
        sv_conf = self.__t_meta['config']
        if sv_conf.is_file():
            sv_conf_backup = Path('{}/{}.backup'.format(os.getcwd(), sv_conf.name))
            copy2(sv_conf, sv_conf_backup)
            self.__t_meta['config-backup'] = sv_conf_backup

    def __t_restore_sv_conf__(self):
        if self.__t_meta['config-backup'] is not None:
            self.__t_cp_to_sv_conf_loc__(self.__t_meta['config-backup'])
            os.remove(self.__t_meta['config-backup'])

    def __t_cp_to_sv_conf_loc__(self, path: Path):
        copy2(path, self.__t_meta['config'])

    def __test_on_t__(self):
        supervisor = Path("/etc/init.d/nemea-supervisorl")
        if not supervisor.is_file():
            pass
            # return

        self.__t_backup_sv_conf__()
        self.__t_cp_to_sv_conf_loc__(self.__t_meta['test-config'])

        input("Press Enter to continue...")

        self.__t_restore_sv_conf__()

    def run(self):
        if self.__t:
            self.__test_on_t__()



def main():
    parser = argparse.ArgumentParser(description='Auto testing for NEMEA-SIoT.')
    parser.add_argument('-m', metavar='M', type=str, nargs='+',
                        help='specify module to be tested')

    parser.add_argument('-n', choices='TC', action='append',
                        help='do not [C]ompile / [T]est')

    auto_test = IntegrationTest()
    parser.parse_args(namespace=auto_test)

    auto_test.run()


if __name__ == '__main__':
    main()
