#! /usr/bin/env python


from sys import argv
import os
import stat
import MeCab


def main(t):
    cb = os.popen("xsel").read()
    path = os.getenv("XDG_CONFIG_HOME") + "/kepwing/control"
    if not stat.S_ISFIFO(os.stat(path)[stat.ST_MODE]):
        return

    m = MeCab.Tagger()
    cb = m.parseToNode(cb).next.feature.split(',')[6]
    print cb

    f = open(path, "w")
    if t == "popup":
        f.write("popup %s\n" % cb)
    elif t == "window":
        f.write("window %s\n" % cb)


    f.close()


if __name__ == '__main__':
    main(argv[1])








