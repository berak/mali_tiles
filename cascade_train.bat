set NSTAGES=64
if not '%1'=='' set NSTAGES=%1
set METHOD=LBP
if not '%2'=='' set METHOD=%2
E:\code\opencv242\bin\Release\opencv_traincascade.exe -data data -vec haar_vec.txt -bg haar_bg.txt -numPos 300 -numNeg 200 -featureType %METHOD% -mode ALL -numStages %NSTAGES%
