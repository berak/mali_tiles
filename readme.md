ongoing attempts to spot houses on satelite imagery in mali, 
in the hope to help the Humanitarian OpenStreetMap Team.

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
  ( one of the better attemps so far, it suffered from me using whole (positive)tiles as input)

* cascade.cpp, cascade_create.bat, cascade_train.bat:
  train a haar/lbp cascade on it, and use that for detection
  

* birds.cpp/ipcap.cpp are just socket download helpers



<pre><code>


some preliminary results from the feature detection (ml.cpp):
=== Run information ===

Scheme:       weka.classifiers.trees.RandomForest -I 10 -K 0 -S 1
Test mode:    10-fold cross-validation


================================================================================================================

train_crowd_harris_sift_3.arff  // that is: harris detector, sift descriptor, 3*more negative that positive samples

Correctly Classified Instances       59258               73.4318 %
Incorrectly Classified Instances     21440               26.5682 %
Total Number of Instances            80698     

TP Rate   FP Rate   Precision   Recall  F-Measure   Class
  0.907     0.788      0.777     0.907     0.837    0
  0.212     0.093      0.43      0.212     0.284    1

     a     b       classified as
 55001  5633 |     a = 0
 15807  4257 |     b = 1

================================================================================================================

train_crowd_FAST_SIFT_4.arff

Correctly Classified Instances       65706               86.156  %
Incorrectly Classified Instances     10558               13.844  %
Total Number of Instances            76264     

TP Rate   FP Rate   Precision   Recall  F-Measure   Class
  0.964     0.549      0.875     0.964     0.918    0
  0.451     0.036      0.759     0.451     0.566    1

     a     b       classified as
 58834  2184 |     a = 0
  8374  6872 |     b = 1

================================================================================================================

train_crowd_GFTT_SIFT_5.arff

Correctly Classified Instances       59028               76.6239 %
Incorrectly Classified Instances     18008               23.3761 %
Total Number of Instances            77036     

TP Rate   FP Rate   Precision   Recall  F-Measure   Class
  0.92      0.857      0.813     0.92      0.863    0
  0.143     0.08       0.307     0.143     0.195    1

     a     b       classified as
 56845  4928 |     a = 0
 13080  2183 |     b = 1

=========================================================================================================

train_crowd_SIFT_SIFT_5.arff

Correctly Classified Instances       23677               84.158  %
Incorrectly Classified Instances      4457               15.842  %
Total Number of Instances            28134     

TP Rate   FP Rate   Precision   Recall  F-Measure   Class
  0.971     0.809      0.858     0.971     0.911    0
  0.191     0.029      0.57      0.191     0.287    1

     a     b       classified as
 22782   676 |     a = 0
  3781   895 |     b = 1

=========================================================================================================

train_crowd_SURF_SURF_5.arff  

Correctly Classified Instances       50635               83.2265 %
Incorrectly Classified Instances     10205               16.7735 %
Total Number of Instances            60840     

TP Rate   FP Rate   Precision   Recall  F-Measure   Class
  0.987     0.942      0.84      0.987     0.907    0
  0.058     0.013      0.472     0.058     0.104    1

     a     b       classified as
 50044   662 |     a = 0
  9543   591 |     b = 1


=========================================================================================================

train_crowd_MSER_SURF_2.arff

Correctly Classified Instances       13602               83.1977 %
Incorrectly Classified Instances      2747               16.8023 %
Total Number of Instances            16349     

TP Rate   FP Rate   Precision   Recall  F-Measure   Class
  0.923     0.349      0.841     0.923     0.88     0
  0.651     0.077      0.808     0.651     0.721    1

     a     b       classified as
 10060   844 |     a = 0
  1903  3542 |     b = 1

=========================================================================================================

train_crowd_MSER_SURF_3.arff

Correctly Classified Instances       18582               85.2542 %
Incorrectly Classified Instances      3214               14.7458 %
Total Number of Instances            21796     

TP Rate   FP Rate   Precision   Recall  F-Measure   Class
  0.946     0.428      0.869     0.946     0.906    0
  0.572     0.054      0.779     0.572     0.659    1

     a     b       classified as
 15470   881 |     a = 0
  2333  3112 |     b = 1




=========================================================================================================
added data, 2600 tiles seen, 794 containing positive rects

train_crowd_MSER_SURF_3.arff

Correctly Classified Instances       32320               84.4217 %
Incorrectly Classified Instances      5964               15.5783 %
Total Number of Instances            38284     


TP Rate   FP Rate   Precision   Recall  F-Measure   Class
  0.94      0.443      0.864     0.94      0.9      0
  0.557     0.06       0.755     0.557     0.641    1

     a     b       classified as
 26986  1728 |     a = 0
  4236  5334 |     b = 1



</code></pre>
