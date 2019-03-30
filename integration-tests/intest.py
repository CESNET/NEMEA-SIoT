import configparser
import os
import signal
import subprocess
import sys
import threading
import time
import json
import re
from shutil import which

################################################################################
# SETTINGS #####################################################################

class DEF:
    SV_SOCKET = 'intest-sock'
    SV_CONFIG = 'siot-supervisor-config.xml'
    SV_LOG_DIR = 'logs/'
    IFC_BO_DRIVER_STATS = 'u:beeeOnEvent-driverStats'
    IFC_BO_NODE_STATS = 'u:beeeOnEvent-nodeStats'
    IFC_BO_HCI_STATS = 'u:beeeOnEvent-HCIStats'
    IFC_BO_EXPORT = 'u:beeeOnEvent-export'
    IFC_HCI_COLLECTOR = 'u:siot-bleStats'
    NM_LOGGER_OUTPUT = 'testing.out'


class TESTS: # interface / file to inject / file to check
    ALL = [
        (DEF.IFC_BO_DRIVER_STATS, 'recordings/z-wave-dos-driver.log', None),
        (DEF.IFC_BO_HCI_STATS, 'recordings/ble-dos.log', None),
        (None, None, 'expected.out'),
    ]


################################################################################
# UTILS ########################################################################

class Printer:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'
    B_ORG = '\033[100m'

    @staticmethod
    def print_warning(what: str) -> None:
        print(Printer.WARNING + what + Printer.ENDC)

    @staticmethod
    def print_error_output(what: str) -> None:
        print(Printer.B_ORG + what + Printer.ENDC)


class Progress:
    animation = ['[-  ]', '[ - ]', '[  -]', '[ - ]',]

    def __init__(self, message: str, print_result=True):
        self.__message = message
        self.__print_result = print_result
        self.__done = threading.Event()
        self.__inProgress = threading.Thread(target=self.__print_in_progress__,
                                             args=(self.__done,))
        self.__inProgress.start()

    def __del__(self):
        self.__finished__()

    def __finished__(self):
        self.__done.set()
        self.__inProgress.join()

    def __print_in_progress__(self, finished_event: threading.Event):
        i = 0
        while not finished_event.is_set():
            sys.stdout.write('\r')
            sys.stdout.write(self.__message)
            sys.stdout.write(self.animation[i % len(self.animation)])
            sys.stdout.flush()
            i += 1
            time.sleep(0.2)

    def __print_result__(self, succ: bool):

        if not self.__print_result:
            cnt = len(self.__message) + len(self.animation)
            sys.stdout.write('\r{}'.format(' ' * cnt))
            sys.stdout.flush()
            return

        color = Printer.BOLD

        if succ:
            color += Printer.OKGREEN
            sign = '✔'
        else:
            color += Printer.FAIL
            sign = '✘'

        sys.stdout.write('\r{}  {}  {}{}\n'.format(color, sign, self.__message, Printer.ENDC))
        sys.stdout.flush()

    def succeed(self):
        self.__finished__()
        self.__print_result__(True)

    def failed(self):
        self.__finished__()
        self.__print_result__(False)

    def finish(self, return_code: int):
        if return_code == 0:
            self.succeed()
        else:
            self.failed()


class Shell:
    def __init__(self, directory='.'):
        self.out = None
        self.err = None
        self.code = None
        self.caption = 'Task caption'
        self.__process = subprocess.Popen('/bin/bash', cwd=directory,
                                          stdin=subprocess.PIPE,
                                          stdout=subprocess.PIPE,
                                          stderr=subprocess.PIPE)

    def __del__(self):
        if self.__process.returncode is None:
            self.__process.kill()

    def execute(self, command: str) -> bool:
        return self.execute_array([command])

    def execute_array(self, commands: []) -> bool:
        p = Progress(self.caption)
        for cmd in commands:
            self.__process.stdin.write(str.encode(cmd + '\n'))

        out, err = self.__process.communicate()
        self.out = out.decode()
        self.err = err.decode()
        self.code = self.__process.returncode
        p.finish(self.code)

        return self.code == 0

    def get_pid(self):
        return self.__process.pid

################################################################################
# MAIN LOGIC ###################################################################

class NemeaLikeSys:
    def __init__(self, basic, config):
        self.basic = basic
        self.__wd = basic.get('directory')
        can_install = basic.getboolean('install')
        self.enabled = config.getboolean('enabled')
        self.__shouldInstall = config.getboolean('install') and can_install
        self.__shouldUninstall = config.getboolean('uninstall') and can_install
        self.__shouldCompile = config.getboolean('compile')
        self.__repo = config.get('repo')
        self.__name = config.get('name')
        self.__binary = config.get('binary')
        self.__repoDir = None

    def __del__(self):
        if self.__shouldUninstall:
            if self.__shouldCompile:
                self.__uninstall_source_code()

    def __clone_repo(self):
        x = Shell(self.__wd)
        x.caption = 'cloning {0} repository into "{1}/{0}"'.format(self.__name,
                                                                   self.__wd)
        x.execute(
            'git clone --recursive {} {}'.format(self.__repo, self.__name))
        return '{}/{}'.format(self.__wd, self.__name)

    def __compile(self):
        x = Shell(self.__repoDir)
        x.caption = 'compiling {}'.format(self.__name)
        x.execute_array([
            './bootstrap.sh',
            './configure',
            'make -j3',
        ])

    def __install_from_source_code(self):
        x = Shell(self.__repoDir)
        x.caption = 'installing {} from source code'.format(self.__name)
        x.execute('make install')

    def __uninstall_source_code(self):
        x = Shell(self.__repoDir)
        x.caption = 'uninstalling {} from source code'.format(self.__name)
        x.execute('make uninstall')

    def __install_from_package(self):
        pass

    def __uninstall_package(self):
        pass

    def handle(self) -> None:
        if not self.enabled:
            return

        if self.__shouldCompile:
            self.__repoDir = self.__clone_repo()
            self.__compile()

            if self.__shouldInstall:
                self.__install_from_source_code()

        elif self.__shouldInstall:
            self.__install_from_package()


def create_dir(name: str) -> bool:
    x = Shell()
    x.caption = 'creating directory "' + name + '"'
    return x.execute('mkdir ' + name)


def delete_dir(name: str) -> bool:
    x = Shell()
    x.caption = 'removing directory "' + name + '"'
    return x.execute('rm -rf ' + name)

def start_process(popen_array: []) -> subprocess.Popen:
    if which(popen_array[0]) is None:
        raise EnvironmentError('{} cannot be launched'.format(popen_array[0]))

    return subprocess.Popen(popen_array, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

class Tester:
    def __init__(self, config):

        pass
        # self.config = config
        # self.basic = config['basic']
        # self.wd = self.basic.get('directory')
        # self.__createdDir = False
        # self.__createdDir = create_dir(self.wd)
        # self.__supervisor_starter = None
        if os.getuid() != 0:
            raise EnvironmentError('root privileges are demanded')

        self.__supervisor_pid = None

    def __del__(self):
        pass
        self.__stop_supervisor()
        # if self.__createdDir and self.basic.getboolean('clean'):
        #     delete_dir(self.wd)

    def __start_supervisor(self):
        # supervisor have to have log directory, and if given directory does
        # not exist, it ends with a failure
        if not os.path.isdir(DEF.SV_LOG_DIR):
            create_dir(DEF.SV_LOG_DIR)

        # following subprocess will start supervisor in daemon mode
        # but the process will not be the daemon itself it will just start
        # the supervisor and end
        sv_starter = start_process(['supervisor', '-d',
                                    '-T', DEF.SV_CONFIG,
                                    '-L', DEF.SV_LOG_DIR,
                                    '-s', DEF.SV_SOCKET,])

        # PID of the daemon itself can be found in the starter process output
        # which looks like:
        # > Fri Mar 22 09:40:09 2019 [INFO] PID of daemon process: 23160.
        out = sv_starter.stdout.readline().decode()
        regex = re.compile('PID of daemon process: ([0-9]*)')
        self.__supervisor_pid = int(regex.findall(out)[0])

    def __stop_supervisor(self):
        if self.__supervisor_pid is None:
            return

        # SIGTERM kills the supervisor and all the processes it started
        os.kill(int(self.__supervisor_pid), signal.SIGTERM)

    def __get_supervisor_data(self) -> json:
        #try if supervisor is running
        os.kill(self.__supervisor_pid, 0)

        cli = start_process(['supervisor_cli', '-i', '-s', DEF.SV_SOCKET,])
        cli_out, cli_err = cli.communicate()
        return json.loads(cli_out)

    @staticmethod
    def __print_modules_statuses(data: json):
        for i in data:
            status = data[i]['status']
            status_color = Printer.FAIL

            if status == 'running':
                status_color = Printer.OKGREEN

            print('{}  {:<22} {} ->  {} {}'.format(Printer.OKBLUE, i,
                                                   status_color, status,
                                                   Printer.ENDC))

    @staticmethod
    def __do_log_replay(interface: str, file: str):
        if interface is None:
            if file is None:
                return

            raise RuntimeError('missing file or interface')

        p = Progress('injecting data form {} to IFC {}'.format(file, interface))

        logreplay = start_process(['logreplay', '-n', '-i', interface, '-f', file])
        logreplay.communicate()

        p.finish(logreplay.returncode)

    @staticmethod
    def __compare_files(file1: str, file2: str):
        if file1 is None:
            return

        p = Progress('comparing files {} and {}'.format(file1, file2))
        diff = start_process(['diff', file1, file2])
        diff.communicate()
        p.finish(diff.returncode)

        return diff.returncode

    def __run_tests(self):
        for i in TESTS.ALL:
            self.__do_log_replay(i[0], i[1])
            p = Progress('waiting after log replay', False)
            time.sleep(3)
            p.succeed()
            self.__compare_files(i[2], DEF.NM_LOGGER_OUTPUT)

    def run(self) -> None:
        self.__start_supervisor()
        time.sleep(1)
        self.__print_modules_statuses(self.__get_supervisor_data())
        self.__run_tests()


def main():
    config = configparser.ConfigParser()
    config.read('example.ini')
    tester = Tester(config)
    tester.run()


if __name__ == '__main__':
    try:
        main()
    except Exception as e:
        print(e.args)
