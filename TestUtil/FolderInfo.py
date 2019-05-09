import os


class FolderInfo:
    def __init__(self, path):
        info = next(os.walk(path))
        self.name = info[0]
        self.dirs = info[1]
        self.files = sorted(info[2])
