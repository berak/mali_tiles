ongoing attempts to spot houses on satelite imagery in mali, 
in the hope to help the Humanitarian OpenStreetMap Team.

* tms2quad.py:
  script to convert quadid's to x,y,zoom and back, (thanks, povaddict!)

* main.cpp:
  given zoomlevel, x, y, and patchcount, download patchcount*patchcount tiles
 
* mark.cpp:
  make positive and negative lists for such a tileset ( manually accepting/rejecting tiles) same idea as the crowdsource page

* contours.cpp
  try to find ( and match ) contours
  ( could not threshold due to low contrast, had to use a canny for binarization, which chopped it up far too much to get useful outlines)
  
* ml.cpp:
  use sift/orb features, train a random forest with descriptors and try to classify.
  writing those out to arff made it possible to throw weka / orange and such at it.
  ( one of the better attemps so far, it suffered from me using whole (positive)tiles as input)

* eigen.cpp:
  abusing face recognition
  ( one of the better attemps so far, it suffered from me using whole (positive)tiles as input)

* cascade.cpp, cascade_create.bat, cascade_train.bat:
  train a haar/lbp cascade on it, and use that for detection
  

* birds.cpp/ipcap.cpp are just socket download helpers
