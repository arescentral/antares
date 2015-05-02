#!/usr/bin/env python

import argparse
import contextlib
import multiprocessing.pool
import os
import shutil
import subprocess
import sys
import tempfile


def run(name, cmd):
    sub = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    output, _ = sub.communicate()
    if sub.returncode != 0:
        sys.stderr.write("%s: %s failed:\n" % (name, os.path.basename(cmd[0])))
        sys.stderr.write("====================\n")
        sys.stderr.write(output)
        sys.stderr.write("====================\n")
        return False
    return True


def unit_test(opts, name, args=[]):
    return run(name, ["out/cur/%s" % name] + args)


def diff_test(name, cmd, expected):
    with NamedTemporaryDir() as d:
        return (run(name, cmd + ["--output=%s" % d]) and
                run(name, ["diff", "-ru", "-x.*", expected, d]))


def data_test(opts, name, args=[]):
    return diff_test(name, ["out/cur/%s" % name] + args, "test/%s" % name)


def offscreen_test(opts, name, args=[]):
    cmd = ["out/cur/offscreen", name]
    if opts.smoke:
        cmd.append("--text")
        expected = "test/smoke/%s" % name
    else:
        expected = "test/%s" % name
    return diff_test(name, cmd + args, expected)


def replay_test(opts, name, args=[]):
    cmd = ["out/cur/replay", "test/%s.NLRP" % name, "--text"]
    if opts.smoke:
        cmd.append("--smoke")
        expected = "test/smoke/%s" % name
    else:
        expected = "test/%s" % name
    return diff_test(name, cmd + args, expected)


def call(args):
    return args[0](*args[1:])


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--smoke", action="store_true")
    opts = parser.parse_args()

    pool = multiprocessing.pool.ThreadPool()
    result = pool.map_async(call, [
        (unit_test, opts, "fixed-test"),

        (data_test, opts, "build-pix"),
        (data_test, opts, "object-data"),
        (data_test, opts, "shapes"),
        (data_test, opts, "tint"),

        (offscreen_test, opts, "main-screen"),
        (offscreen_test, opts, "mission-briefing", ["--text"]),
        (offscreen_test, opts, "options"),
        (offscreen_test, opts, "pause", ["--text"]),

        (replay_test, opts, "and-it-feels-so-good"),
        (replay_test, opts, "astrotrash-plus"),
        (replay_test, opts, "blood-toil-tears-sweat"),
        (replay_test, opts, "hand-over-fist"),
        (replay_test, opts, "hornets-nest"),
        (replay_test, opts, "make-way"),
        (replay_test, opts, "moons-for-goons"),
        (replay_test, opts, "out-of-the-frying-pan"),
        (replay_test, opts, "shoplifter-1"),
        (replay_test, opts, "space-race"),
        (replay_test, opts, "the-left-hand"),
        (replay_test, opts, "the-mothership-connection"),
        (replay_test, opts, "the-stars-have-ears"),
        (replay_test, opts, "while-the-iron-is-hot"),
        (replay_test, opts, "yo-ho-ho"),
        (replay_test, opts, "you-should-have-seen-the-one-that-got-away"),
    ])
    pool.close()
    result.wait()
    if all(result.get()):
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
