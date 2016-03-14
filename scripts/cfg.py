#!/usr/bin/env


from __future__ import division, print_function, unicode_literals
import contextlib
import os
import subprocess
import sys

def platform():
    if sys.platform == "darwin":
        return "mac"
    for platform in ["linux", "win"]:
        if sys.platform.startswith(platform):
            return platform
    return "unknown"


def tint(text, color):
    if not (color and os.isatty(1)):
        return text
    color = {
        "red": 31,
        "green": 32,
        "yellow": 33,
        "blue": 34,
    }[color]
    return "\033[1;%dm%s\033[0m" % (color, text)


@contextlib.contextmanager
def step(message):
    sys.stdout.write(message + "...")
    sys.stdout.flush()
    padding = ((27 - len(message)) * " ")
    def msg(failure, color=None):
        print(padding + tint(failure, color))
        msg.called = True
    msg.called = False
    yield msg
    if not msg.called:
        print(padding + tint("ok", "green"))


def check_bin(cmdline, what=None, input=None):
    if what is None:
        what = cmdline[0]
    with step("checking for %s" % what) as msg:
        stdin = None
        if input is not None:
            stdin = subprocess.PIPE
        try:
            p = subprocess.Popen(cmdline, stdin=stdin, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            p.communicate(input)
            if p.returncode == 0:
                return True
        except OSError:
            pass
        msg("missing", color="red")
        return False


def check_pkg(lib):
    with step("checking for %s" % lib) as msg:
        try:
            p = subprocess.Popen(["pkg-config", lib])
            p.communicate()
            if p.returncode == 0:
                return True
        except OSError:
            pass
        msg("missing", color="red")
        return False


def makedirs(path):
    try:
        os.makedirs(path)
    except OSError as e:
        if e.errno != 17:
            raise


def linkf(dst, src):
    try:
        os.unlink(src)
    except OSError as e:
        pass
    os.symlink(dst, src)
