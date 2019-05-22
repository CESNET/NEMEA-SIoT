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

import re

from .ProgressPrinter import ProgressPrinter
from .FolderInfo import FolderInfo


# This class serves to scan the NEMEA-SIoT repository and provide information
# about its content. Its behavior in any other directory was not tested and
# thus can be considered as undefined. Current version can only provide
# information about C modules, despite the fact that it has already defined
# interface for the Python modules info.
class RepoScanner:
    def __init__(self, repo: str = '.'):
        p = ProgressPrinter('Scanning repository')
        f = FolderInfo(repo)

        self.modules = {'c': {}, 'python': {}}
        for d in f.dirs:
            d_info = FolderInfo(d)
            if 'bootstrap.sh' in d_info.files:
                self.modules['c'][d_info.name] = {}
                self.modules['c'][d_info.name]['got-tests'] = 'tests' in d_info.dirs
                self.modules['c'][d_info.name]['test-config'] = 'auto-test.json' in d_info.files

                if self.modules['c'][d_info.name]['got-tests']:
                    test_files = FolderInfo('{}/{}'.format(d, 'tests')).files
                    self.modules['c'][d_info.name]['tests'] = self.__get_test_pairs__(test_files)

        p.succeed()

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

    # Returns information whether a module with the given name is present in
    # the repository.
    def got_module(self, m: str) -> bool:
        return m in self.modules['c'].keys() or m in self.modules[
            'python'].keys()

    # Returns true if the given a module with the given name is present in
    # the repository and considered as a C module. Raises an exception, if
    # a module with given name is not present.
    def is_c_module(self, m: str) -> bool:
        if not self.got_module(m):
            raise RuntimeError('no such module')

        return m in self.modules['c'].keys()

    # Returns true if the given a module with the given name is present in
    # the repository and considered as a Python module. Raises an exception, if
    # a module with given name is not present.
    def is_python_module(self, m: str) -> bool:
        if not self.got_module(m):
            raise RuntimeError('no such module')

        return m in self.modules['python'].keys()

    # Returns true if the given a module with the given name is present in
    # the repository and its directory contains directory named 'tests'
    # Raises an exception, if a module with given name is not present.
    def got_tests(self, m: str) -> bool:
        if not self.got_module(m):
            raise RuntimeError('no such module')

        if self.is_c_module(m):
            return self.modules['c'][m]['got-tests']

        if self.is_python_module(m):
            return self.modules['python'][m]['got-tests']

    # Returns true if the given a module's directory contains file named
    # 'auto-test.json'. Raises an exception, if a module with given name is
    # not present.
    def got_test_config(self, m: str) -> bool:
        if not self.got_module(m):
            raise RuntimeError('no such module')

        if self.is_c_module(m):
            return self.modules['c'][m]['test-config']

        if self.is_python_module(m):
            return self.modules['python'][m]['test-config']

    # Returns array of names of the tests present in the given modules tests
    # directory. Raises an exception, if a module with given name is not present
    # or does not include directory named 'tests'.
    def get_tests(self, m: str) -> []:
        if not self.got_tests(m):
            raise RuntimeError('no does not have tests')

        if self.is_c_module(m):
            return self.modules['c'][m]['tests']

        if self.is_python_module(m):
            return self.modules['python'][m]['tests']
