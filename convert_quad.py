#!/usr/bin/python

import sys
import math


def deg2num(lat_deg, lon_deg, zoom):
    lat_rad = math.radians(lat_deg)
    n = 2.0 ** zoom
    xtile = int((lon_deg + 180.0) / 360.0 * n)
    ytile = int((1.0 - math.log(math.tan(lat_rad) + (1 / math.cos(lat_rad))) / math.pi) / 2.0 * n)
    return (xtile, ytile)
  
def num2deg(xtile, ytile, zoom):
    n = 2.0 ** zoom
    lon_deg = xtile / n * 360.0 - 180.0
    lat_rad = math.atan(math.sinh(math.pi * (1 - 2 * ytile / n)))
    lat_deg = math.degrees(lat_rad)
    return (lat_deg, lon_deg) 


def tms2quad(x,y,zoom):
    quadkey=""
    for i in reversed(range(zoom)):
        num = 0
        mask = 1<<i
        if x & mask:
            num |= 1
        if y & mask:
            num |= 2
        quadkey += str(num)

    return quadkey

def quad2tms(quadkey):
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
    return x, y, zoom



if __name__ == "__main__":
    
    if len(sys.argv)<2:
        print "supported commands:"
        print
        print " tms2quad x y zoom        -> quad"
        print " quad2tms quad            -> x,y,zoom"
        print
        print " latlon2quad lat lon zoom -> quad"
        print " quad2latlon quad         -> lat,lon"
    else:
        cmd = sys.argv[1]
        if cmd == "tms2quad":
            print tms2quad(int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4]) )
        elif cmd == "quad2tms":
            print quad2tms(sys.argv[2])
        elif cmd == "latlon2quad":
            z = int(sys.argv[4])
            x,y = deg2num(float(sys.argv[2]), float(sys.argv[3]), z)
            print tms2quad( x,y, z )
        elif cmd == "quad2latlon":
            x,y,z = quad2tms(sys.argv[2])
            print num2deg(x,y,z)


