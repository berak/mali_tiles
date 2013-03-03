ongoing attempts to spot houses on satelite imagery in mali, 
in the hope to help the Humanitarian OpenStreetMap Team.

the current (training) strategy:
  - extract lat/lon of tiles from croudsource.gpx
  - make quadids zoom l7 from that
  - get the img
  - manually (!) mark smaller 64x64 positive rects, that contain (mostly, or relevant) houses
    * that's a positive tile
    * i need to restrict my feature search to regions, that contain mostly positive entries,
      so every click adds a fixed region to sample for positives
  - any tile, that does not contain a positive region, gets marked as negative
  - collect 2d-feature descriptors like SIFT,SURF, OCB, the like from the img
    * see which combo of feature detectors and descriptors works best for my information 
  - collect as many positive descriptors from that, and like 3 * more negative descriptors
  - train a random forest on that ( in weka, 10xfold cross ref )
  

the benefit from that will be, that the actual test might look as simple as:
  - setup the tree, load the classifier
  - for each candidate tile, just call the prediction
  
===
Files:


* convert_quad.py:
  script to convert quadid's to x,y,zoom and back, latlon, too. (thanks, povaddict!)

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

* cascade.cpp, cascade_create.bat, cascade_train.bat:
  train a haar/lbp cascade on it, and use that for detection
  

* birds.cpp/ipcap.cpp are just socket download helpers



