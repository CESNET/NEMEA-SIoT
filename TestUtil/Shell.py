import subprocess


class Shell:
    def __init__(self, directory='.'):
        self.out = None
        self.err = None
        self.code = None
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
        for cmd in commands:
            self.__process.stdin.write(str.encode(cmd + '\n'))

        out, err = self.__process.communicate()
        self.out = out.decode()
        self.err = err.decode()
        self.code = self.__process.returncode

        return self.code == 0

    def get_pid(self):
        return self.__process.pid

    def succeed(self) -> bool:
        return self.code == 0
