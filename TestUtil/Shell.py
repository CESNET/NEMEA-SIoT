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

import subprocess


# This class serves as a bash console abstraction. It opens up /bin/bash
# as a subprocess and can be used to execute a single bash command as well
# as multiple commands. However, it can be used for only one such execution.
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

    # This method serves to execute a single bash command.
    def execute(self, command: str) -> bool:
        return self.execute_array([command])

    # This method serves to execute multiple bash commands in an array
    # of strings.
    def execute_array(self, commands: []) -> bool:
        if self.code is not None:
            raise RuntimeError('Shell could only be used once')

        for cmd in commands:
            self.__process.stdin.write(str.encode(cmd + '\n'))

        out, err = self.__process.communicate()
        self.out = out.decode()
        self.err = err.decode()
        self.code = self.__process.returncode

        return self.code == 0

    # Returns PID of the bash subprocess
    def get_pid(self):
        return self.__process.pid

    # Returns true if the last executed command succeeded
    def succeed(self) -> bool:
        return self.code == 0
