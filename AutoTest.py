from TestUtil.SIoTNemeaTester import SIoTNemeaTester
from TestUtil.Shell import Shell
from argparse import ArgumentParser
import json
import subprocess


class AutoTest(SIoTNemeaTester):
    @staticmethod
    def __get_expected_executable__(m: str) -> str:
        return './siot-{0}'.format(m)

    def run(self) -> int:
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

            for t in tests:
                if self.quit:
                    return 2

                self.__test_started__(m, t)

                f_in = self.__get_test_in_path__(m, t)
                f_out = self.__get_test_out_path__(m, t)
                f_exp = self.__get_test_expected_out_path__(m, t)
                ifc_in = 'u:test-in'
                ifc_out = 'u:test-out'

                x = Shell(directory=m)
                x.execute_array(pre_shell)

                # starts the module with appropriate arguments for testing plus
                # with potential arguments defined in the auto-tests.json file
                module = subprocess.Popen(
                    [self.__get_expected_executable__(m),
                     '-i', '{},{}'.format(ifc_in, ifc_out)] + run_args,
                     stderr=subprocess.PIPE, stdout=subprocess.PIPE, cwd=m)

                self.__inject_and_capture__(ifc_in, f_in, ifc_out, f_out)
                x = Shell(directory=m)
                x.execute_array(post_shell)

                self.__test_ended__(self.__compare_files__(f_exp, f_out))

        return self.__finished__()


def main():
    parser = ArgumentParser(description='Auto testing for NEMEA-SIoT.')

    auto_test = AutoTest(parser)
    exit(auto_test.run())


if __name__ == '__main__':
    try:
        main()
    except Exception as e:
        print('\n[!] ERROR: {}'.format(e))
        exit(3)
