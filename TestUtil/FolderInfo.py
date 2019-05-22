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

import os


# This simple class serves as an abstraction giving more user-friendly form
# to the output of os.walk method.
class FolderInfo:
    def __init__(self, path):
        info = next(os.walk(path))
        self.name = info[0]
        self.dirs = info[1]
        self.files = sorted(info[2])
