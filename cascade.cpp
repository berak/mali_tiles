/////////////////////////////////////////////////////////////////////////
// THIS GIVE SEGFAULT
/////////////////////////////////////////////////////////////////////////
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/contrib/contrib.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <fstream>
 
using namespace std;
using namespace cv;

// http://ecn.t0.tiles.virtualearth.net/tiles/a03331213223211110.jpeg?g=1146
// http://ecn.t0.tiles.virtualearth.net/tiles/a0333130302322200000.jpeg?g=1146
// http://binged.it/X5EA2n
// http://binged.it/X5EIPq


std::vector<std::string> readdir( const char * dmask ) 
{
	std::vector<std::string> vec;
	HANDLE hFind;
	WIN32_FIND_DATA FindFileData;
	if ((hFind = FindFirstFile(dmask, &FindFileData)) != INVALID_HANDLE_VALUE)
	{
		do {
			vec.push_back( FindFileData.cFileName );
		} while(FindNextFile(hFind, &FindFileData));
		FindClose(hFind);
	}
	return vec;
}

int main(int argc, char **argv)
{
	const char *faceCascadeFilename = "data/cascade.xml";
	cv::CascadeClassifier cascade;
	if ( ! cascade.load( faceCascadeFilename ) )
		return -1;

	namedWindow("im",0);
	vector<string> fn = readdir("tilesc/*.jpeg");
	for ( size_t i=0; i<fn.size(); i++  )
	{
		Mat mali_img = imread(string("tilesc/") + fn[i]);
		if ( mali_img.empty() )
			continue;
		Mat grey; cvtColor(mali_img,grey,CV_RGB2GRAY);
		std::vector<cv::Rect> faces;
		cascade.detectMultiScale(grey, faces, 1.1, 3, 0|CV_HAAR_SCALE_IMAGE, cv::Size(60, 60), cv::Size(70, 70) );
		for (size_t j=0; j<faces.size(); ++j) 
		{
			cv::rectangle(mali_img, faces[j].tl(), faces[j].br(), cv::Scalar(0,255,0), 2);
		}
		cerr << i << " " << faces.size() << endl;

		imshow("im",mali_img);
		int k = waitKey();
		if ( k==27 )
			break;
	}
}

