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
        self.__turris = True
        self.__turris_meta = {'config': Path("/etc/config/nemea-supervisor")}
        # self.__turris_meta = {'config': Path(
        #     "/home/xhosal00/school/bp/impl/siot-nemea/TestUtil/nemea-supervisor.makak.conf")}

    def __turris_backup_sv_conf__(self):
        sv_conf = self.__turris_meta['config']
        if sv_conf.is_file():
            sv_conf_backup = '{}/{}.backup'.format(os.getcwd(), sv_conf.name)
            copy2(sv_conf, sv_conf_backup)
            self.__turris_meta['config-backup'] = sv_conf_backup

    def __turris_restore_sv_conf__(self):
        if self.__turris_meta['config-backup'] is not None:
            copy2(self.__turris_meta['config-backup'], self.__turris_meta['config'])
            os.remove(self.__turris_meta['config-backup'])

    def __test_on_turris__(self):
        supervisor = Path("/etc/init.d/nemea-supervisorl")
        if not supervisor.is_file():
            pass
            # return

        self.__turris_backup_sv_conf__()
        self.__turris_restore_sv_conf__()

    def run(self):
        if self.__turris:
            self.__test_on_turris__()



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
