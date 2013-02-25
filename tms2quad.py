#!/usr/bin/python

import sys

cmd = sys.argv[1]

if cmd == "tms2quad":
    zoom=int(sys.argv[2])
    x=int(sys.argv[3])
    y=int(sys.argv[4])

    quadkey=""
    for i in reversed(range(zoom)):
        num = 0
        mask = 1<<i
        if x & mask:
            num |= 1
        if y & mask:
            num |= 2
        quadkey += str(num)

    print quadkey

elif cmd == "quad2tms":
    quadkey = sys.argv[2]
    zoom = len(quadkey)
    x = y = 0
    for n in quadkey:
        num = int(n)
        x *= 2
        y *= 2
        if num & 1:
            x += 1
        if num & 2:
            y += 1
    print zoom, x, y
