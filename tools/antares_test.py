#!/usr/bin/env python

from __future__ import with_statement
import contextlib
import shutil
import subprocess
import tempfile
from waflib.Utils import to_list
from waflib.Configure import conf

def options(opt):
    opt.load("test", tooldir="ext/waf-sfiera")

def configure(cnf):
    cnf.load("test", tooldir="ext/waf-sfiera")

def build(bld):
    bld.load("test", tooldir="ext/waf-sfiera")


class AntaresTestCase(object):

    def __init__(self, bld, target, args, srcs, expected):
        self.target = target
        self.args = to_list(args)
        self.srcs = [bld.path.find_resource(s) for s in to_list(srcs)]
        self.binary = bld.path.find_or_declare(self.args[0])
        self.expected = bld.path.find_dir(expected)

    def execute(self, tst, log):
        with NamedTemporaryDir() as dir:
            antares_command = (
                    [self.binary.abspath()] + self.args[1:] +
                    [s.abspath() for s in self.srcs] + ["--output=%s" % dir])
            tst.to_log(antares_command)
            antares = subprocess.Popen(antares_command, stdout=log, stderr=log)
            antares.communicate()
            assert antares.returncode == 0, "Antares failed"

            diff_command = ["diff", "-ru", self.expected.abspath(), dir]
            tst.to_log(diff_command)
            diff = subprocess.Popen(diff_command, stdout=log, stderr=log)
            diff.communicate()
            assert diff.returncode == 0, "diff failed"


@conf
def antares_test(bld, target, rule, expected, srcs=[]):
    if hasattr(bld, "test_cases"):
        bld.test_cases[target] = AntaresTestCase(bld, target, rule, srcs, expected)


@contextlib.contextmanager
def NamedTemporaryDir():
    dir = tempfile.mkdtemp()
    try:
        yield dir
    finally:
        shutil.rmtree(dir)
