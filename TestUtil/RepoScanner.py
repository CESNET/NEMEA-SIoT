import re


from .FolderInfo import FolderInfo


# This class serves to scan the NEMEA-SIoT repository and provide information
# about its content. Its behavior in any other directory was not tested and
# thus can be considered as undefined. Current version can only provide
# information about C modules, despite the fact that it has already defined
# interface for the Python modules info.
class RepoScanner:
    def __init__(self, repo: str = '.'):
        f = FolderInfo(repo)

        self.modules = {}
        for d in f.dirs:
            d_info = FolderInfo(d)
            if 'bootstrap.sh' in d_info.files:
                self.modules[d_info.name] = {}
                self.modules[d_info.name]['got-tests'] = 'tests' in d_info.dirs
                self.modules[d_info.name]['test-config'] = 'auto-test.json' in d_info.files

                if self.modules[d_info.name]['got-tests']:
                    test_files = FolderInfo('{}/{}'.format(d, 'tests')).files
                    self.modules[d_info.name]['tests'] = self.__get_tests_ins_outs__(test_files)

    def filter(self, modules: []):
        if not modules:
            return

        self.modules = { k:v for k,v in self.modules.items() if k in modules }

        for m in modules:
            if m not in self.modules.keys():
                raise RuntimeError('no such module: {}'.format(m))

    @staticmethod
    def __get_tests_ins_outs__(files):
        tests = {}
        file_re = re.compile(r'^(.*)\.([0-9])\.(csv|out)$')

        for f in files:
            match = file_re.match(f)

            if not match:
                continue

            (name, ifc, ext) = match.groups()

            if name not in tests.keys():
                tests[name] = {}
                tests[name]['in'] = []
                tests[name]['out'] = []

            if ext == 'csv':
                tests[name]['in'].append(ifc)
            else:
                tests[name]['out'].append(ifc)

        return tests


    # Returns information whether a module with the given name is present in
    # the repository.
    def got_module(self, m: str) -> bool:
        return m in self.modules.keys()

    # Returns true if the given a module with the given name is present in
    # the repository and its directory contains directory named 'tests'
    # Raises an exception, if a module with given name is not present.
    def got_tests(self, m: str) -> bool:
        if not self.got_module(m):
            raise RuntimeError('no such module: {}'.format(m))

        return self.modules[m]['got-tests']

    # Returns true if the given a module's directory contains file named
    # 'auto-test.json'. Raises an exception, if a module with given name is
    # not present.
    def got_test_config(self, m: str) -> bool:
        if not self.got_module(m):
            raise RuntimeError('no such module: {}'.format(m))

        return self.modules[m]['test-config']


    # Returns array of names of the tests present in the given modules tests
    # directory. Raises an exception, if a module with given name is not present
    # or does not include directory named 'tests'.
    def get_tests(self, m: str) -> []:
        if not self.got_tests(m):
            raise RuntimeError('no does not have tests')

        return self.modules[m]['tests']
