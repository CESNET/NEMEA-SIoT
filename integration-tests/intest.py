import configparser
import os
import subprocess
import sys
import threading
import time


class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


class Shell:
    animation = ['   ', '.  ', '.. ', '...']

    def __init__(self, directory='.'):
        self.out = None
        self.err = None
        self.code = None
        self.__done = None
        self.__inProgress = None
        self.caption = 'Task caption'
        self.__process = subprocess.Popen('/bin/bash', cwd=directory,
                                          stdin=subprocess.PIPE,
                                          stdout=subprocess.PIPE,
                                          stderr=subprocess.PIPE)

    def __del__(self):
        if self.__process.returncode is None:
            self.__process.kill()

    def __started(self):
        self.__done = threading.Event()
        self.__inProgress = threading.Thread(target=self.__print_in_progress,
                                             args=(self.__done,))
        self.__inProgress.start()

    def __finished(self):
        self.__done.set()
        self.__inProgress.join()
        self.__print_result()

    def __print_in_progress(self, finished_event: threading.Event):
        i = 0
        while not finished_event.is_set():
            sys.stdout.write('\r')
            sys.stdout.write(self.caption)
            sys.stdout.write(Shell.animation[i % len(Shell.animation)])
            sys.stdout.flush()
            i += 1
            time.sleep(0.3)

    def __print_result(self):
        sys.stdout.write('\r')
        color = bcolors.OKGREEN

        if self.code != 0:
            color = bcolors.FAIL
        color += bcolors.BOLD
        sys.stdout.write(color + self.caption + bcolors.ENDC + '\n')
        sys.stdout.flush()

    def execute(self, command: str) -> None:
        self.execute_array([command])

    def execute_array(self, commands: []) -> None:
        self.__started()
        for cmd in commands:
            self.__process.stdin.write(str.encode(cmd + '\n'))

        out, err = self.__process.communicate()
        self.out = out.decode()
        self.err = err.decode()
        self.code = self.__process.returncode
        self.__finished()


class NemeaLikeSys:
    def __init__(self, basic, config):
        self.basic = basic
        self.__wd = basic.get('directory')
        can_install = basic.getboolean('install')
        self.enabled = config.getboolean('enabled')
        self.__shouldInstall = config.getboolean('install') and can_install
        self.__shouldCompile = config.getboolean('compile')
        self.__repo = config.get('repo')
        self.__name = config.get('name')
        self.__binary = config.get('binary')

    def __clone_repo(self):
        x = Shell(self.__wd)
        x.caption = 'cloning {0} repository into "{1}/{0}"'.format(self.__name, self.__wd)
        x.execute('git clone --recursive {} {}'.format(self.__repo, self.__name))
        return '{}/{}'.format(self.__wd, self.__name)

    def __compile(self, repo: str):
        x = Shell(repo)
        x.caption = 'compiling {}'.format(self.__name)
        x.execute_array([
            './bootstrap.sh',
            './configure',
            'make -j3',
        ])

    def __install_from_source_code(self, repo: str):
        x = Shell(repo)
        x.caption = 'installing {} from source code'.format(self.__name)
        x.execute('make install')

    def __uninstall_source_code(self, repo: str):
        x = Shell(repo)
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
            repo_dir = self.__clone_repo()
            self.__compile(repo_dir)

            if self.__shouldInstall:
                self.__install_from_source_code(repo_dir)

        elif self.__shouldInstall:
            self.__install_from_package()


def create_dir(name: str) -> None:
    x = Shell()
    x.caption = 'creating directory "' + name + '"'
    x.execute('mkdir ' + name)


def delete_dir(name: str) -> None:
    x = Shell()
    x.caption = 'removing directory "' + name + '"'
    x.execute('rm -rf ' + name)


def print_warning(what: str) -> None:
    print(bcolors.WARNING + what + bcolors.ENDC)


class Tester:
    def __init__(self, config):
        self.config = config
        self.basic = config['basic']
        self.wd = self.basic.get('directory')
        create_dir(self.wd)

    def __del__(self):
        if self.basic.getboolean('clean'):
            delete_dir(self.wd)

    def run(self) -> None:
        root = os.getuid() == 0
        installation_enabled = self.basic.getboolean('install')
        run_system_enabled = self.basic.getboolean('startSystem')

        if not root and installation_enabled:
            raise PermissionError('installation enabled - needs root privilege')

        if not root and run_system_enabled:
            raise PermissionError('system run enabled - needs root privilege')

        nemea = NemeaLikeSys(self.basic, self.config['nemea'])
        siot = NemeaLikeSys(self.basic, self.config['nemea-siot'])
        nemea.handle()
        siot.handle()

        if run_system_enabled:
            from shutil import which

            if which('supervisor') is not None:
                create_dir('logs')
                print('You can now run the system using:\n'
                      '    supervisor -T siot_config.xml -L logs/')


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
