# -*- coding: utf-8 -*-

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

import threading
import sys
import time

from .Colors import Colors


# This class serve to print information that a task is in progress and also
# to inform if it succeeded. When an instance is created, the given message
# is printed along with the animation which indicates that the task is in
# progress.
class ProgressPrinter:
    animation = ['[-  ]', '[ - ]', '[  -]', '[ - ]']

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
            sys.stdout.write('\r{}{}{}{}'.format(Colors.CYAN, self.animation[i % len(self.animation)], self.__message, Colors.RESET))
            sys.stdout.flush()
            i += 1
            time.sleep(0.2)

    def __print_result__(self, succ: bool):

        if not self.__print_result:
            cnt = len(self.__message) + len(self.animation)
            sys.stdout.write('\r{}'.format(' ' * cnt))
            sys.stdout.flush()
            return

        color = ''

        if succ:
            color += Colors.GREEN
            sign = '✔'
        else:
            color += Colors.RED
            sign = '✘'

        sys.stdout.write('\r{}  {}  {}{}\n'.format(color, sign, self.__message, Colors.RESET))
        sys.stdout.flush()

    # This method should be called if the task succeeded. It prints out the
    # message with along with the check sing.
    def succeed(self):
        self.__finished__()
        self.__print_result__(True)

    # This method should be called if the task failed. It prints out the
    # message with along with the X sing.
    def failed(self):
        self.__finished__()
        self.__print_result__(False)

    # This method should be called if it is demanded to stop the animation,
    # but the indication about task success is not required.
    def stop(self):
        self.__finished__()
        print('\n')

    # This method determines whether the task succeeded based on a given return
    # code and sequentially calls succeed of failed method.
    def finish(self, return_code: int):
        if return_code == 0:
            self.succeed()
        else:
            self.failed()
