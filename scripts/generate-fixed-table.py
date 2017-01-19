#!/usr/bin/env python
# Copyright (C) 2017 The Antares Authors
# This file is part of Antares, a tactical space combat game.
# Antares is free software, distributed under the LGPL+. See COPYING.

table = {}
for i in xrange(1000):
    flt = i / 1000.0
    fix = int(round(flt * 256))
    rep = ("%03d" % i).rstrip("0") or "0"
    err = abs(flt - (fix / 256.0))
    if fix in table:
        other_rep, other_err = table[fix]
        if len(other_rep) < len(rep):
            continue  # Don't replace a shorter representation
        elif len(rep) < len(other_rep):
            pass  # Always replace a longer representation
        else:  # If it's the same size,
            if other_err < err:
                continue  # then don't replace a more accurate representation
    table[fix] = rep, err
    
print "static const char fractions[][5] = {"
for _, (rep, _) in sorted(table.iteritems()):
    print '    ".%s",' % rep
print "};"
