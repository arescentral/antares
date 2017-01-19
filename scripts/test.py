#!/usr/bin/env python
# Copyright (C) 2017 The Antares Authors
# This file is part of Antares, a tactical space combat game.
# Antares is free software, distributed under the LGPL+. See COPYING.

import argparse
import collections
import contextlib
import cStringIO
import multiprocessing.pool
import os
import shutil
import subprocess
import sys
import tempfile
import time
import traceback


START = "START"
PASSED = "PASSED"
FAILED = "FAILED"
EXCEPT = "EXCEPT"


def run(queue, name, cmd):
    sub = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    output, _ = sub.communicate()
    if sub.returncode != 0:
        print("%s failed:\n%s" % (os.path.basename(cmd[0]), output))
        return False
    return True


def unit_test(opts, queue, name, args=[]):
    return run(queue, name, ["out/cur/%s" % name] + args)


def diff_test(queue, name, cmd, expected):
    with NamedTemporaryDir() as d:
        return (run(queue, name, cmd + ["--output=%s" % d]) and
                run(queue, name, ["diff", "-ru", "-x.*", expected, d]))


def data_test(opts, queue, name, args=[], smoke_args=[]):
    if opts.smoke:
        args += smoke_args
        expected = "test/smoke/%s" % name
    else:
        expected = "test/%s" % name
    return diff_test(queue, name, ["out/cur/%s" % name] + args, expected)


def offscreen_test(opts, queue, name, args=[]):
    cmd = ["out/cur/offscreen", name]
    if opts.smoke:
        cmd.append("--text")
        expected = "test/smoke/%s" % name
    else:
        expected = "test/%s" % name
    return diff_test(queue, name, cmd + args, expected)


def replay_test(opts, queue, name, args=[]):
    cmd = ["out/cur/replay", "test/%s.NLRP" % name, "--text"]
    if opts.smoke:
        cmd.append("--smoke")
        expected = "test/smoke/%s" % name
    else:
        expected = "test/%s" % name
    return diff_test(queue, name, cmd + args, expected)


def call(args):
    fn = args[0]
    opts = args[1]
    queue = args[2]
    name = args[3]
    args = list(args[4:])

    sys.stdout = cStringIO.StringIO()

    queue.put((name, START,))
    try:
        start = time.time()
        result = fn(opts, queue, name, *args)
        end = time.time()
        if result:
            queue.put((name, PASSED, end - start, sys.stdout.getvalue()))
        else:
            queue.put((name, FAILED, end - start, sys.stdout.getvalue()))
    except:
        end = time.time()
        print(traceback.format_exc())
        queue.put((name, EXCEPT, end - start, sys.stdout.getvalue()))


def main():
    if sys.platform.startswith("linux"):
        if "DISPLAY" not in os.environ:
            # TODO(sfiera): determine when Xvfb is unnecessary and skip this.
            print("no DISPLAY; using Xvfb")
            os.execvp("xvfb-run",
                      ["xvfb-run", "-s", "-screen 0 640x480x24"] + sys.argv)

    test_types = "unit data offscreen replay".split()
    parser = argparse.ArgumentParser()
    parser.add_argument("--smoke", action="store_true")
    parser.add_argument("-t", "--type", action="append", choices=test_types)
    parser.add_argument("test", nargs="*")
    opts = parser.parse_args()

    queue = multiprocessing.Queue()
    pool = multiprocessing.pool.ThreadPool()
    tests = [
        (unit_test, opts, queue, "fixed-test"),

        (data_test, opts, queue, "build-pix", [], ["--text"]),
        (data_test, opts, queue, "object-data"),
        (data_test, opts, queue, "shapes"),
        (data_test, opts, queue, "tint"),

        (offscreen_test, opts, queue, "main-screen"),
        (offscreen_test, opts, queue, "mission-briefing", ["--text"]),
        (offscreen_test, opts, queue, "options"),
        (offscreen_test, opts, queue, "pause", ["--text"]),

        (replay_test, opts, queue, "and-it-feels-so-good"),
        (replay_test, opts, queue, "astrotrash-plus"),
        (replay_test, opts, queue, "blood-toil-tears-sweat"),
        (replay_test, opts, queue, "hand-over-fist"),
        (replay_test, opts, queue, "hornets-nest"),
        (replay_test, opts, queue, "make-way"),
        (replay_test, opts, queue, "moons-for-goons"),
        (replay_test, opts, queue, "out-of-the-frying-pan"),
        (replay_test, opts, queue, "shoplifter-1"),
        (replay_test, opts, queue, "space-race"),
        (replay_test, opts, queue, "the-left-hand"),
        (replay_test, opts, queue, "the-mothership-connection"),
        (replay_test, opts, queue, "the-stars-have-ears"),
        (replay_test, opts, queue, "while-the-iron-is-hot"),
        (replay_test, opts, queue, "yo-ho-ho"),
        (replay_test, opts, queue, "you-should-have-seen-the-one-that-got-away"),
    ]

    if opts.test:
        test_map = dict((t[3], t) for t in tests)
        tests = [test_map[test] for test in opts.test]

    if opts.type:
        if "unit" not in opts.type:
            tests = [t for t in tests if t[0] != unit_test]
        if "data" not in opts.type:
            tests = [t for t in tests if t[0] != data_test]
        if "offscreen" not in opts.type:
            tests = [t for t in tests if t[0] != offscreen_test]
        if "replay" not in opts.type:
            tests = [t for t in tests if t[0] != replay_test]

    sys.stderr.write("Running %d tests:\n" % len(tests))
    start = time.time()
    result = pool.map_async(call, tests)
    pool.close()

    failed = handle_queue(queue, tests)

    end = time.time()
    sys.stderr.write("\nRan %d tests in %.2fs\n" % (len(tests), end - start))
    if failed:
        sys.stderr.write("%d tests failed.\n" % failed)
        sys.exit(1)
    else:
        sys.stderr.write("All tests passed!\n")


def handle_queue(queue, tests):
    pending = len(tests)
    failed = 0
    completed = []
    in_progress = collections.OrderedDict()
    while pending:
        msg = queue.get()
        for _ in in_progress:
            sys.stderr.write("\033[1A\033[2K")
        name, cmd, params = msg[0], msg[1], msg[2:]
        if cmd in START:
            print_name = name
            if len(print_name) > 36:
                print_name = name[:33] + "..."
            in_progress[name] = "  %-40s ...\n" % print_name
        elif cmd in [PASSED, FAILED, EXCEPT]:
            del in_progress[name]
            duration, output = params
            if cmd == PASSED:
                color = 2
            else:
                failed += 1
                color = 1
                sys.stderr.write("%s failed:\n" % name)
                sys.stderr.write("====================\n")
                sys.stderr.write(output)
                sys.stderr.write("====================\n")
            rstr = "\033[1;38;5;%dm%s\033[0m" % (color, cmd)
            print_name = name
            if len(print_name) > 36:
                print_name = name[:33] + "..."
            sys.stderr.write("  %-40s %s in %0.2fs\n" % (print_name, rstr, duration))
            pending -= 1
        for line in in_progress.values():
            sys.stderr.write(line)
    return failed


@contextlib.contextmanager
def NamedTemporaryDir():
    dir = tempfile.mkdtemp()
    try:
        yield dir
    finally:
        shutil.rmtree(dir)


if __name__ == "__main__":
    main()
