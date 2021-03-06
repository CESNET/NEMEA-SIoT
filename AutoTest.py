from TestUtil.SIoTNemeaTester import SIoTNemeaTester
from TestUtil.Shell import Shell
from argparse import ArgumentParser
import json
from subprocess import Popen, PIPE


class AutoTest(SIoTNemeaTester):
    @staticmethod
    def __get_expected_executable__(m: str) -> str:
        return './siot-{0}'.format(m)

    def run(self) -> int:
        valgrind = []
        if self.v:
            valgrind = ['valgrind', '--error-exitcode=42']

        if self.apvo and not self.v:
            self.parser.error('apvo agrument is dependand on v argument')

        self.__prepare_test_output_directories__(self.repo.modules)
        for m in self.repo.modules:
            if not self.repo.got_tests(m):
                continue

            tests = self.repo.get_tests(m)

            pre_shell = []
            post_shell = []
            run_args = []

            # if a module directory contains file named auto-tests.json, this file
            # is parsed
            if self.repo.got_test_config(m):
                with open('{}/auto-test.json'.format(m)) as json_file:
                    data = json.load(json_file)

                pre_shell = data['pre-shell']
                post_shell = data['post-shell']
                run_args = data['run-args']

            for t in tests.items():
                name, ifcs = t

                if self.quit:
                    return 2

                self.__test_started__(m, t)

                files_in = self.__get_test_in_paths__(m, t)
                files_out = self.__get_test_out_paths__(m, t)
                files_exp = self.__get_test_expected_out_paths__(m, t)

                ifc_in = 'u:in{}'.format(',u:in'.join(ifcs['in']))
                ifc_out = 'u:out{}'.format(',u:out'.join(ifcs['out']))

                x = Shell(directory=m)
                x.execute_array(pre_shell)

                # starts the module with appropriate arguments for testing plus
                # with potential arguments defined in the auto-tests.json file
                module = Popen(
                     valgrind + [self.__get_expected_executable__(m),
                     '-i', '{},{}'.format(ifc_in, ifc_out)] + run_args,
                     stderr=PIPE, stdout=PIPE, cwd=m)

                self.__inject_and_capture__(ifcs['in'], files_in, ifcs['out'], files_out)
                x = Shell(directory=m)
                x.execute_array(post_shell)

                self.__test_ended__(self.__compare_files__(files_exp, files_out))
                out, err = module.communicate()
                if module.returncode == 42 or (self.v and self.apvo):
                    print(err.decode())

        return self.__finished__()


def main():
    parser = ArgumentParser(description='Auto testing for NEMEA-SIoT.')
    parser.add_argument('-v', action='store_true', help='test with valgrind')
    parser.add_argument('--apvo', action='store_true', help='always print valgrind output')
    auto_test = AutoTest(parser)
    exit(auto_test.run())


if __name__ == '__main__':
    try:
        main()
    except Exception as e:
        print('\n[!] ERROR: {}'.format(e))
        exit(3)
