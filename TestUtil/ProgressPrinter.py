# -*- coding: utf-8 -*-

import threading
import sys
import time

from .Colors import Colors


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
            sys.stdout.write('\r{}{} {}{}'.format(Colors.CYAN, self.animation[i % len(self.animation)], self.__message, Colors.RESET))
            sys.stdout.flush()
            i += 1
            time.sleep(0.2)

    def __print_result__(self, succ: bool):

        if not self.__print_result:
            cnt = len(self.__message) + len(self.animation)
            sys.stdout.write('\r{}'.format(' ' * cnt))
            sys.stdout.flush()
            return

        # color = Colors.BOLD
        color = ''

        if succ:
            color += Colors.GREEN
            sign = '✔'
        else:
            color += Colors.RED
            sign = '✘'

        sys.stdout.write('\r{}  {}  {}{}\n'.format(color, sign, self.__message, Colors.RESET))
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
