#!/usr/bin/env python

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


def unit_test(name, args=[]):
    return run(name, ["out/cur/%s" % name] + args)


def diff_test(name, cmd, expected):
    with NamedTemporaryDir() as d:
        return (run(name, cmd + ["--output=%s" % d]) and
                run(name, ["diff", "-ru", "-x.*", expected, d]))


def data_test(name, args=[]):
    return diff_test(name, ["out/cur/%s" % name] + args, "test/%s" % name)


def offscreen_test(name, args=[]):
    cmd = ["out/cur/offscreen", name]
    if bool(os.environ.get("SMOKE", "")):
        cmd.append("--text")
        expected = "test/smoke/%s" % name
    else:
        expected = "test/%s" % name
    return diff_test(name, cmd + args, expected)


def replay_test(name, args=[]):
    cmd = ["out/cur/replay", "test/%s.NLRP" % name, "--text"]
    if bool(os.environ.get("SMOKE", "")):
        cmd.append("--smoke")
        expected = "test/smoke/%s" % name
    else:
        expected = "test/%s" % name
    return diff_test(name, cmd + args, expected)


def call(args):
    return args[0](*args[1:])


def main():
    args = sys.argv[1:]
    assert not sys.argv[1:]

    pool = multiprocessing.pool.ThreadPool()
    result = pool.map_async(call, [
        (unit_test, "fixed-test"),

        (data_test, "build-pix"),
        (data_test, "object-data"),
        (data_test, "shapes"),
        (data_test, "tint"),

        (offscreen_test, "main-screen"),
        (offscreen_test, "mission-briefing", ["--text"]),
        (offscreen_test, "options"),
        (offscreen_test, "pause", ["--text"]),

        (replay_test, "and-it-feels-so-good"),
        (replay_test, "astrotrash-plus"),
        (replay_test, "blood-toil-tears-sweat"),
        (replay_test, "hand-over-fist"),
        (replay_test, "hornets-nest"),
        (replay_test, "make-way"),
        (replay_test, "moons-for-goons"),
        (replay_test, "out-of-the-frying-pan"),
        (replay_test, "shoplifter-1"),
        (replay_test, "space-race"),
        (replay_test, "the-left-hand"),
        (replay_test, "the-mothership-connection"),
        (replay_test, "the-stars-have-ears"),
        (replay_test, "while-the-iron-is-hot"),
        (replay_test, "yo-ho-ho"),
        (replay_test, "you-should-have-seen-the-one-that-got-away"),
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
