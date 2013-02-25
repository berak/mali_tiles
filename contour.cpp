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
 
using namespace std;
using namespace cv;

//
// 17 64443 59342 40


// http://ecn.t0.tiles.virtualearth.net/tiles/a03331213223211110.jpeg?g=1146
// http://ecn.t0.tiles.virtualearth.net/tiles/a0333130302322200000.jpeg?g=1146
// http://binged.it/X5EA2n
// http://binged.it/X5EIPq


int main(int argc, char *argv[]) 
{
	std::string path ="tiles/a03331123332113322.jpeg";
	if ( argc>1 ) 
	{
		path = argv[1];
	}

	cv::Mat mali_img = cv::imread(path);
	cv::Mat img = mali_img;
	cv::Mat rez;
	resize(mali_img, rez,Size(mali_img.rows*2,mali_img.cols*2),0,0,INTER_CUBIC);

	Mat txx;
    cv::cvtColor(rez,img,CV_RGB2GRAY);

	//equalizeHist(img,img);
//	bilateralFilter(txx,img,3, 30, 110);

	Mat canny_output;
	vector<vector<Point> > contours, good;

	vector<Vec4i> hierarchy;

	/// Detect edges using canny
	int thresh = 80;
	Canny( img, canny_output, thresh, thresh*2, 3 );
	//Canny( img, img, thresh, thresh*2, 3 );
	//threshold(img,canny_output,120,255,4);
	/// Find contours  
	findContours( canny_output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_TC89_KCOS, Point(0, 0) );

	RNG rng;
	/// Draw contours

	cv::namedWindow("im0",1);
	cv::namedWindow("im1",1);
	cv::namedWindow("im2",1);
	imshow("im0", mali_img);
	imshow("im1", canny_output);

	for( int i = 0; i< contours.size(); i++ )
	{ 
		Mat draw = rez.clone();
		Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
		drawContours( draw, contours, i, color, 1, 8, hierarchy, 0, Point() ); 
		int k = waitKey();
		if ( k != ' ' )
		{
			cerr << i << endl;
			good.push_back(contours[i]);
		}
		imshow("im2", draw);
		//Mat contour;
		//cv::approxPolyDP(cv::Mat(contours[i]), contour, 3, true);
		//const cv::Point * pc = contour.ptr<cv::Point>(0);
		//int nc = contour.rows;
		//cv::polylines( mali_img, &pc, &nc, 1, 0, color );
	}
	//cerr << good.size() << endl;
	//cv::FileStorage fs(cv::format("contours.yml",item.c_str()),cv::FileStorage::WRITE);
	//for( int i = 0; i< good.size(); i++ )
	//{ 
	//	fs << format("K%d",i) << good[i];
	//}

	//cv::FileStorage fs(cv::format("contours.yml",item.c_str()),cv::FileStorage::READ);
	//for( int i = 0; i<1000; i++ )
	//{ 
	//	vector < Point > v;
	//	fs[format("K%d",i)] >> v;
	//	if ( v.empty() )
	//		break;
	//	good.push_back(v);
	//}

	for( int i = 0; i<contours.size(); i++ )
	{ 
		for ( int j=0; j<good.size(); j++ )
		{
			double d = matchShapes( contours[i], good[j],CV_CONTOURS_MATCH_I2, 0 );
			//cerr << i << " " << j << " " << d << endl;
			if ( d < 0.5 )
			{
				Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
				const cv::Point * pc = &(good[j][0]);
				int nc = good[j].size();
				//const cv::Point * pc = &(contours[i][0]);
				//int nc = contours[i].size();
				cv::polylines( mali_img, &pc, &nc, 1, 0, color );
			}
		}
	}

	waitKey(0);
	return 0;
}
