ongoing attempts to spot houses on satelite imagery in mali, 
in the hope to help OSM mappers.

* tms2quad.py
  script to convert quadid's to x,y,zoom and back, (thanks, povaddict!)

* main.cpp
  given zoomlevel, x, y, and patchcount, download patchcount*patchcount tiles
 
* mark.cpp
  make positive and negative lists for such a tileset ( manually accepting/rejecting tiles)

* contours.cpp
  try to find ( and match ) contours
  
* ml.cpp
  use sift/orb features, train a random forest with descriptors and try to classify

* eigen.cpp
  abusing face recognition

* cascade.cpp
  train a haar/lbp cascade on it, and use that for detection
  

birds/ipcap are just socket download helpers
