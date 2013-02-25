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
#include "ipcap.h"
 
using namespace std;
using namespace cv;

//
// 17 64443 59342 40
// 17 65791 59776
//
// 16 32804 29827
//

// http://ecn.t0.tiles.virtualearth.net/tiles/a03331213223211110.jpeg?g=1146
// http://ecn.t0.tiles.virtualearth.net/tiles/a0333130302322200000.jpeg?g=1146
// http://binged.it/X5EA2n
// http://binged.it/X5EIPq


//
//void segmentImage(const cv::Mat input, cv::Mat &output, int k)
//{
//    cv::Mat temp(input.reshape(input.channels(), input.rows * input.cols));
//    cv::Mat bestLabels;
//    cv::Mat centers;
//    cv::Mat tempf;
//	temp.convertTo(tempf, CV_32FC3);
//
//    double err = cv::kmeans(tempf, k, bestLabels,
//            cv::TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 10, 1.0),
//            3, cv::KMEANS_PP_CENTERS, centers);
//	cout << err << endl;
//
//    ////debug
//    //for (int z = 0; z < centers.rows; z++)
//    //{
//    //      std::cout << "Main color " << z << std::endl;
//    //      std::cout << "R " << centers.at<float>(z,0) << std::endl;
//    //      std::cout << "G " << centers.at<float>(z,1) << std::endl;
//    //      std::cout << "B " << centers.at<float>(z,2) << std::endl;
//    //}
//
//	output = cv::Mat(input.rows,input.cols,CV_8U);
//	int * best = bestLabels.ptr<int>(0);
//	cv::Vec3f *cen  = centers.ptr<cv::Vec3f>(0);
//    for (int y = 0; y < output.rows; y++)
//    {
//		// get rowpointer (pic might not be continuous)
//		//cv::Vec3f *out = output.ptr<cv::Vec3f>(y);
//		uchar *out = output.ptr<uchar>(y);
//        for (int x = 0; x < output.cols; x++)
//        {
//			cv::Vec3f c = cen[*best++];
//			*out++ = uchar( (int(c[0])>>2) +(int(c[1])>>1) + (int(c[2])>>2) ); 
//        }
//    }
//}
//


//
// http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames
//

const float M_PI = 3.14159265358979323846f;

int lon2tilex(double lon, int z) 
{ 
	return (int)(floor((lon + 180.0) / 360.0 * pow(2.0, z))); 
}
 
int lat2tiley(double lat, int z)
{ 
	return (int)(floor((1.0 - log( tan(lat * M_PI/180.0) + 1.0 / cos(lat * M_PI/180.0)) / M_PI) / 2.0 * pow(2.0, z))); 
}
 
double tilex2lon(int x, int z) 
{
	return x / pow(2.0, z) * 360.0 - 180;
}
 
double tiley2lat(int y, int z) 
{
	double n = M_PI - 2.0 * M_PI * y / pow(2.0, z);
	return 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
}

std::string tz( int zoom, int x, int y )
{
	string quadkey="";
	for (int i=zoom-1; i>=0; i--)
	{
		int num = 0;
		int mask = 1<<i;
		if ( x & mask )
			num |= 1;
		if ( y & mask )
			num |= 2;
		quadkey += ('0' + num);
	}
	cerr << zoom << " " << x << " " << y << " " <<  quadkey << endl;
	return quadkey;
}


Mat load(string item, string folder)
{
	string path= folder + string("/a") + item + string(".jpeg");
	Mat img = imread(path);
	if ( img.empty() )
	{
		IpCapture cap;
		string host("ecn.t0.tiles.virtualearth.net");
		string fin(".jpeg?g=1145");
		string ini("/tiles/a");
		//string host("http://ecn.dynamic.t2.tiles.virtualearth.net");
		//string fin("?mkt=de-de&it=A,G,L&shading=hill&og=2&n=z");
		//std::string ini("/comp/ch/");

		string uri = ini + item + fin;
		img = cap.load_jpeg( host.c_str(), uri.c_str() );
		if ( img.empty() )
		{
			cerr << "no image : " << uri <<  endl;
		} 
		else 
		{
			cv::imwrite( path, img );
		}
	}
	return img;
}

void check_crowd()
{
	int zoom = 17;
	namedWindow("im1",1);
	ifstream inp("mali-crowd.txt");
	while(!inp.eof())
	{
		double lat,lon;
		inp >> lat;
		inp >> lon;
		int name=-1;
		inp >> name;
		if ( name == -1 )
			break;

		cerr << lat << " " << lon << " " << name << endl;
		int y = lat2tiley(lat,zoom);
		int x = lon2tilex(lon,zoom);
		string item = tz(zoom,x,y);
		Mat img = load(item, "tilesc");
		if ( ! img.empty())
		{
			imshow("im1",img);
		}
		if ( waitKey(100) == 27 )
		{
			return ;
		}
	}
}

void download(int z, int xx, int yy, int n )
{
	for ( int x=xx; x<xx+n; x++ )
	{
		for ( int y=yy; y<yy+n; y++ )
		{
			string item = tz(z,x,y);
			load(item, "tiles");
		}
	}
}

int main(int argc, char *argv[]) 
{

	check_crowd();
	return 0;


	int z  = 17; //13; 
	int n  = 30;
	int xx = 65549;//3932;
	int yy = 59514;//3787;
	if ( argc>1) z = atoi(argv[1]);
	if ( argc>2) xx = atoi(argv[2]);
	if ( argc>3) yy = atoi(argv[3]);
	if ( argc>4) n = atoi(argv[4]);
	download(z,xx,yy,n);
	return 0;

	//
	// garbage all the way down ..
	//

 //   cv::initModule_nonfree(); // needed for "SIFT"
	////setWindowProperty("im1", CV_WND_PROP_FULLSCREEN, 1.0);


	//std::string item("033313032313313033");
	//if ( argc>1 ) 
	//{
	//	item = argv[1];
	//}
	//int positive = 0;
	//if ( argc>2 ) 
	//{
	//	positive = atoi(argv[2]);
	//}
	//std::string host("ecn.t0.tiles.virtualearth.net");
	//std::string uri=std::string("tiles/a") + item + std::string(".jpeg");

	////std::cerr << host;
	//std::cerr << uri << std::endl;

	//cv::Mat mali_img = cv::imread(uri);
	//if ( mali_img.empty() )
	//{
	//	IpCapture cap;
	//	std::string url = std::string("/") + uri + "?g=1146";
	//	mali_img = cap.load_jpeg( host.c_str(), url.c_str() );
	//	if ( mali_img.empty() )
	//	{
	//		return -1;
	//	}
	//	cv::imwrite(uri,mali_img);
	//}

	//cv::Mat img = mali_img;
	//cv::Mat rez;
	//resize(mali_img, rez,Size(mali_img.rows*2,mali_img.cols*2));

	//Mat txx;
 //   cv::cvtColor(rez,txx,CV_RGB2GRAY);

	////equalizeHist(img,img);
	//bilateralFilter(txx,img,4, 80, 110);

	//Mat canny_output;
	//vector<vector<Point> > contours, good;

	//vector<Vec4i> hierarchy;

	///// Detect edges using canny
	//int thresh = 40;
	//Canny( img, canny_output, thresh, thresh*2, 3 );
	////Canny( img, img, thresh, thresh*2, 3 );
	////threshold(img,canny_output,120,255,4);
	///// Find contours  
	//findContours( canny_output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_TC89_KCOS, Point(0, 0) );

	//RNG rng;
	///// Draw contours

	//cv::namedWindow("im0",1);
	//cv::namedWindow("im1",1);
	//cv::namedWindow("im2",1);
	//imshow("im0", mali_img);
	//imshow("im1", canny_output);

 //   //vector<Vec4i> lines;
 //   //HoughLinesP(canny_output, lines, 10, CV_PI/180, 50, 50, 30 );
 //   //for( size_t i = 0; i < lines.size(); i++ )
 //   //{
 //   //    Vec4i l = lines[i];
 //   //    line( mali_img, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 1, CV_AA);
 //   //}

	////Mat dst;// = Mat::zeros( img.size(), CV_32FC1 );
	////Mat dst_norm, dst_norm_scaled;

	/////// Detector parameters
	////int blockSize = 4;
	////int apertureSize = 3;
	////double k = 0.1;

	/////// Detecting corners
	////cornerHarris( img, dst, blockSize, apertureSize, k, BORDER_DEFAULT );

	/////// Normalizing
	////normalize( dst, dst_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat() );
	////convertScaleAbs( dst_norm, dst_norm_scaled ); 

	/////// Drawing a circle around corners
	////for ( int j = 0; j < dst_norm.rows ; j++ )
	////{ 
	////	for ( int i = 0; i < dst_norm.cols; i++ )
	////	{
	////		if( (int) dst_norm.at<float>(j,i) > 120 )
	////		{ 
	////			circle( mali_img, Point( i, j ), 1,  Scalar(0,0,255) ); 
	////		}
	////	} 
	////}    

	//if ( positive ) 
	//{
	//	//segmentImage(mali_img,img,8);
	//	//Mat rgb[3]; split(mali_img,rgb); img = rgb[2];
	//	cv::cvtColor(mali_img,img,CV_RGB2GRAY);

	//	cv::Ptr<cv::FeatureDetector>	 detector  = cv::FeatureDetector::create( "HARRIS" );
	//	cv::Ptr<cv::DescriptorExtractor> extractor = cv::DescriptorExtractor::create( "SURF" );

	//	vector<cv::KeyPoint> keys;
	//	detector->detect( img, keys );

	//	cv::Mat desc;
	//	extractor->compute( img, keys, desc );

	//	cout << desc.cols << " " << desc.rows << endl;
	//	cv::FileStorage fs(cv::format("sift/s%s.yml",item.c_str()),cv::FileStorage::WRITE);
	//	fs << "F" << desc;

	//	for ( int i=0; i<keys.size(); i++ )
	//	{
	//		circle(draw,keys[i].pt,3,Scalar(0,255,0));
	//	}
	//}
	//imshow("im2", draw);
	////imshow("im2", mali_img);

	////Mat img = imread("mali1.png"); 
	////Mat templ = imread("mali_t4.png"); 

	/////// Create the result matrix
	////int result_cols = img.cols - templ.cols + 1;
	////int result_rows = img.rows - templ.rows + 1;
	////Mat result( result_cols, result_rows, CV_32FC1 );

	/////// Do the Matching and Normalize
	////int match_method = 0;
	////matchTemplate( img, templ, result, match_method );
	////normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );

	/////// Localizing the best match with minMaxLoc
	////double minVal; double maxVal; Point minLoc; Point maxLoc;
	////Point matchLoc;

	////minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );



	/////// Show me what you got
	////rectangle( img, matchLoc, Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows ), Scalar::all(0), 2, 8, 0 );
	//////rectangle( result, matchLoc, Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows ), Scalar::all(0), 2, 8, 0 );

	//////imshow( "res", result );


	//waitKey(0);
	return 0;
}
