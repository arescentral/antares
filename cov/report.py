#!/usr/bin/env python
# Copyright (C) 2017 The Antares Authors
# This file is part of Antares, a tactical space combat game.
# Antares is free software, distributed under the LGPL+. See COPYING.

from __future__ import absolute_import, division, print_function, unicode_literals

import cgi
import collections
import contextlib
import json
import os
import re
import struct
import sys

FACTORY = os.path.expanduser(
    "~/Library/Application Support/Antares/Scenarios/com.biggerplanet.ares/")

ACTION_KINDS = {
    1: "create",
    2: "sound",
    3: "alter",
    4: "sparks",
    5: "energy",
    6: "land",
    7: "warp",
    8: "message",
    9: "score",
    10: "win",
    11: "kill",
    12: "retarget",
    13: "special",
    14: "pulse",
    15: "beam",
    16: "flash",
    17: "create2",
    18: "detarget",
    19: "keyon",
    20: "keyoff",
    21: "zoom",
    22: "select",
    23: "assume",
}

ALTER_TYPES = {
    0: "health",
    1: "velocity",
    2: "thrust",
    3: "max_thrust",
    4: "max_velocity",
    5: "max_turn_rate",
    6: "location",
    7: "scale",
    8: "beam",
    9: "pulse",
    10: "special",
    11: "energy",
    12: "owner",
    13: "hidden",
    14: "cloak",
    15: "offline",
    16: "spin",
    17: "base",
    18: "condition",
    19: "occupation",
    20: "money",
    21: "age",
    22: "attributes",
    26: "location",
}

STYLE = """
* { font-family: monospace; }
table { border-spacing: 0px; }
td.level, th.level { white-space: nowrap; text-align: center; min-width: 2em; padding: 0; }
.id, .frame, .name, .race, .class, .kind, .what { text-align: left; padding-right: 0.5em; }
.id, .race, .class { text-align: right; }
.name, .kind, .what { text-align: left; }
tr.covered { background: green; color: white; }
tr.uncovered { background: red; color: white; }
tr.unreachable { background: white; color: darkgray; }
td.covered.level { background: lightgreen; color: white; }
td.uncovered.level { background: pink; color: white; }
td.unreachable.level { background: white; color: darkgray; }
""".strip()

ObjectData = collections.namedtuple("ObjectData", "name short_name note race class_ frame".split())
ActionData = collections.namedtuple("ActionData", "kind what".split())


def main():
    level_names = load_level_names()
    objects = load_objects()
    actions = load_actions(objects)
    reachable = load_reachable()
    covered = load_covered(reachable, sys.argv[1:])

    with html():
        with head():
            title("Coverage Report")
            with tag("style"):
                text(STYLE)

        with body():
            h1("Coverage Report")

            with div():
                a("Objects", href="#objects")
            with div():
                a("Actions", href="#actions")

            h2("Objects", id="objects")

            with table():
                with tr():
                    th("#", class_="id")
                    th("F", class_="frame")
                    th("Object", class_="name")
                    th("Class", class_="class")
                    th("Race", class_="race")
                    for level in sorted(reachable):
                        if level is not None:
                            th(level_names[level])

                for i, obj in enumerate(objects):
                    if i in covered[None]["objects"]:
                        class_ = "covered"
                    elif i in reachable[None]["objects"]:
                        class_ = "uncovered"
                    else:
                        class_ = "unreachable"

                    with tr(class_=class_):
                        object_tooltip = "%s: %s" % (obj.name, obj.note)
                        td(i, class_="id")
                        td(obj.frame[0], class_="frame", title=obj.frame)
                        td(obj.short_name, class_="name", title=object_tooltip)
                        td(obj.class_, class_="class")
                        td(obj.race, class_="race")

                        for level in sorted(reachable):
                            if level is None:
                                continue
                            if (i % 10) == 9:
                                content = level_names[level]
                            else:
                                content = ""

                            if i in covered[level]["objects"]:
                                td(content, class_="covered level")
                            elif i in reachable[level]["objects"]:
                                td(content, class_="uncovered level")
                            else:
                                td(content, class_="unreachable level")

            h2("Actions", id="actions")
            with table():
                with tr():
                    th("#", class_="id")
                    th("Kind", class_="kind")
                    th("What", class_="what")
                    for level in sorted(reachable):
                        if level is not None:
                            th(level_names[level])

                for i, act in enumerate(actions):
                    if i in covered[None]["actions"]:
                        class_ = "covered"
                    elif i in reachable[None]["actions"]:
                        class_ = "uncovered"
                    else:
                        class_ = "unreachable"
                    with tr(class_=class_):

                        td(i, class_="id")
                        td(act.kind, class_="kind")
                        td(act.what, class_="what")

                        for level in sorted(reachable):
                            if level is None:
                                continue
                            if (i % 10) == 9:
                                content = level_names[level]
                            else:
                                content = ""

                            if i in covered[level]["actions"]:
                                td(content, class_="covered level")
                            elif i in reachable[level]["actions"]:
                                td(content, class_="uncovered level")
                            else:
                                td(content, class_="unreachable level")


def load_level_names():
    with open(os.path.expanduser(FACTORY + "strings/4600.json")) as f:
        level_names = json.load(f)
        level_names = [l.split("\\i")[1] for l in level_names]
        level_names = [re.sub("[^A-Z0-9]+", "", l) for l in level_names]
        return level_names


def load_objects():
    object_names = json.load(open(os.path.expanduser(FACTORY + "strings/5000.json")))
    object_short_names = json.load(open(os.path.expanduser(FACTORY + "strings/5001.json")))
    object_notes = json.load(open(os.path.expanduser(FACTORY + "strings/5002.json")))
    with open(os.path.expanduser(FACTORY + "objects/500.bsob")) as f:
        object_data = f.read()
        objects = []
        while object_data:
            chunk, object_data = object_data[:318], object_data[318:]
            attributes, class_, race = struct.unpack(">Lll306x", chunk)
            if class_ == -1:
                class_ = ""
            if race == -1:
                race = ""
            if attributes & 0x00000100:
                frame = "Oriented"
            elif attributes & 0x00000080:
                frame = "Animated"
            elif attributes & 0x00000020:
                frame = "Beam"
            else:
                frame = "Device"
            name = object_names[len(objects)]
            short_name = object_short_names[len(objects)]
            note = object_notes[len(objects)]
            objects.append(ObjectData(name, short_name, note, race, class_, frame))
        return objects


def load_actions(objects):
    with open(os.path.expanduser(FACTORY + "object-actions/500.obac")) as f:
        action_data = f.read()
        actions = []
        while action_data:
            chunk, action_data = action_data[:48], action_data[48:]
            kind, what = struct.unpack(">B23xl20x", chunk)
            kind = ACTION_KINDS.get(kind, "")
            if kind == "alter":
                what = ALTER_TYPES[what >> 24]
            elif kind in ["create", "create2"]:
                what = objects[what].short_name
            elif kind == "message":
                what >>= 16
            elif kind != "win":
                what = ""
            actions.append(ActionData(kind, what))
        return actions


def load_reachable():
    reachable = {None: {"objects": set(), "actions": set()}}
    for data in json.load(open("cov/reachable.json")):
        reachable[data["level"] - 1] = {
            "objects": set(data["objects"]),
            "actions": set(data["actions"]),
        }
        reachable[None]["objects"].update(data["objects"])
        reachable[None]["actions"].update(data["actions"])
    return reachable


def load_covered(levels, paths):
    covered = dict((level, {"objects": set(), "actions": set()}) for level in levels)
    for path in paths:
        with open(path) as f:
            data = json.load(f)
            for level in [None, data["level"] - 1]:
                covered[level]["objects"].update(data["objects"])
                covered[level]["actions"].update(data["actions"])
    return covered


indent = ""


def open_tag(tag, **kwds):
    attrs = [" %s=\"%s\"" % (k.strip("_"), cgi.escape(v, quote=True)) for k, v in kwds.iteritems()]
    return "<%s%s>" % (tag, "".join(attrs))


@contextlib.contextmanager
def context_tag(tag, **kwds):
    global indent
    try:
        sys.stdout.write("%s%s\n" % (indent, open_tag(tag, **kwds)))
        indent += "  "
        yield
    finally:
        indent = indent[:-2]
        sys.stdout.write("%s</%s>\n" % (indent, tag))


def tag(tag, *args, **kwds):
    if args:
        content = " ".join(cgi.escape(str(t)) for t in args)
        sys.stdout.write("%s%s%s</%s>\n" % (indent, open_tag(tag, **kwds), content, tag))
    else:
        return context_tag(tag, **kwds)


def text(s):
    global indent
    sys.stdout.write("%s%s\n" % (indent, cgi.escape(s.replace("\n", "\n" + indent))))


html = lambda *args, **kwds: tag("html", *args, **kwds)
head = lambda *args, **kwds: tag("head", *args, **kwds)
title = lambda *args, **kwds: tag("title", *args, **kwds)
body = lambda *args, **kwds: tag("body", *args, **kwds)
h1 = lambda *args, **kwds: tag("h1", *args, **kwds)
h2 = lambda *args, **kwds: tag("h2", *args, **kwds)
table = lambda *args, **kwds: tag("table", *args, **kwds)
tr = lambda *args, **kwds: tag("tr", *args, **kwds)
th = lambda *args, **kwds: tag("th", *args, **kwds)
td = lambda *args, **kwds: tag("td", *args, **kwds)
a = lambda *args, **kwds: tag("a", *args, **kwds)
div = lambda *args, **kwds: tag("div", *args, **kwds)

if __name__ == "__main__":
    main()
