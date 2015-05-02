#!/usr/bin/env python

import argparse
import contextlib
import multiprocessing.pool
import os
import shutil
import subprocess
import sys
import tempfile
import time


def run(lock, name, cmd):
    sub = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    output, _ = sub.communicate()
    if sub.returncode != 0:
        with lock:
            sys.stderr.write("%s: %s failed:\n" % (name, os.path.basename(cmd[0])))
            sys.stderr.write("====================\n")
            sys.stderr.write(output)
            sys.stderr.write("====================\n")
        return False
    return True


def unit_test(opts, lock, name, args=[]):
    return run(lock, name, ["out/cur/%s" % name] + args)


def diff_test(lock, name, cmd, expected):
    with NamedTemporaryDir() as d:
        return (run(lock, name, cmd + ["--output=%s" % d]) and
                run(lock, name, ["diff", "-ru", "-x.*", expected, d]))


def data_test(opts, lock, name, args=[]):
    return diff_test(lock, name, ["out/cur/%s" % name] + args, "test/%s" % name)


def offscreen_test(opts, lock, name, args=[]):
    cmd = ["out/cur/offscreen", name]
    if opts.smoke:
        cmd.append("--text")
        expected = "test/smoke/%s" % name
    else:
        expected = "test/%s" % name
    return diff_test(lock, name, cmd + args, expected)


def replay_test(opts, lock, name, args=[]):
    cmd = ["out/cur/replay", "test/%s.NLRP" % name, "--text"]
    if opts.smoke:
        cmd.append("--smoke")
        expected = "test/smoke/%s" % name
    else:
        expected = "test/%s" % name
    return diff_test(lock, name, cmd + args, expected)


def call(args):
    fn = args[0]
    opts = args[1]
    lock = args[2]
    name = args[3]
    args = list(*args[4:])

    start = time.time()
    result = fn(opts, lock, name, args)
    end = time.time()

    with lock:
        if result:
            rstr = "\033[1;32mPASSED\033[0m"
        else:
            rstr = "\033[1;31mFAILED\033[0m"
        if len(name) > 36:
            name = name[:33] + "..."
        sys.stderr.write("  %-40s %s in %0.2fs\n" % (name, rstr, end - start))
    return result


def main():
    test_types = "unit data offscreen replay".split()
    parser = argparse.ArgumentParser()
    parser.add_argument("--smoke", action="store_true")
    parser.add_argument("-t", "--type", action="append", choices=test_types)
    parser.add_argument("test", nargs="*")
    opts = parser.parse_args()

    lock = multiprocessing.Lock()
    pool = multiprocessing.pool.ThreadPool()
    tests = [
        (unit_test, opts, lock, "fixed-test"),

        (data_test, opts, lock, "build-pix"),
        (data_test, opts, lock, "object-data"),
        (data_test, opts, lock, "shapes"),
        (data_test, opts, lock, "tint"),

        (offscreen_test, opts, lock, "main-screen"),
        (offscreen_test, opts, lock, "mission-briefing", ["--text"]),
        (offscreen_test, opts, lock, "options"),
        (offscreen_test, opts, lock, "pause", ["--text"]),

        (replay_test, opts, lock, "and-it-feels-so-good"),
        (replay_test, opts, lock, "astrotrash-plus"),
        (replay_test, opts, lock, "blood-toil-tears-sweat"),
        (replay_test, opts, lock, "hand-over-fist"),
        (replay_test, opts, lock, "hornets-nest"),
        (replay_test, opts, lock, "make-way"),
        (replay_test, opts, lock, "moons-for-goons"),
        (replay_test, opts, lock, "out-of-the-frying-pan"),
        (replay_test, opts, lock, "shoplifter-1"),
        (replay_test, opts, lock, "space-race"),
        (replay_test, opts, lock, "the-left-hand"),
        (replay_test, opts, lock, "the-mothership-connection"),
        (replay_test, opts, lock, "the-stars-have-ears"),
        (replay_test, opts, lock, "while-the-iron-is-hot"),
        (replay_test, opts, lock, "yo-ho-ho"),
        (replay_test, opts, lock, "you-should-have-seen-the-one-that-got-away"),
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
    result.wait()
    end = time.time()
    results = result.get()
    sys.stderr.write("\nRan %d tests in %.2fs\n" % (len(tests), end - start))
    if all(results):
        sys.stderr.write("All tests passed!\n")
    else:
        sys.stderr.write("Some tests failed.\n")
        sys.exit(1)


@contextlib.contextmanager
def NamedTemporaryDir():
    dir = tempfile.mkdtemp()
    try:
        yield dir
    finally:
        shutil.rmtree(dir)


if __name__ == "__main__":
    main()
