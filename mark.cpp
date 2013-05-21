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

#include "tools.h"

using namespace std;
using namespace cv;


// http://ecn.t0.tiles.virtualearth.net/tiles/a03331213223211110.jpeg?g=1146
// http://ecn.t0.tiles.virtualearth.net/tiles/a0333130302322200000.jpeg?g=1146
// http://binged.it/X5EA2n
// http://binged.it/X5EIPq


//void findPositives()
//{
//	int good = 0;
//	int cnt = 0;
//	ifstream in("pos_.txt");
//	ofstream posi("posi.txt");
//	ofstream nega("nega.txt");
//	while(!in.eof())
//	{
//		string it;
//		in >> it;
//		int p;
//		in >> p;
//		good += p;
//		cnt ++;
//		//cerr << it << " " << p << " " << good << " " << cnt << endl;
//
//		if ( p != 0 )
//		{
//			posi << it << endl;
//		}
//		else
//		{
//			nega << it << endl;
//		}
//	}
//
//	cerr << good << " / " << cnt << endl;
//}
//



//void makneg(int z, int xx, int yy, int n )
//{
//	ifstream pos("pos.txt");
//	map<string,int> items;
//	while( ! pos.eof() )
//	{
//		string s;
//		pos >> s;
//		if ( s.empty() )
//			break;
//		items[s] = 1;
//	}
//	ofstream neg("neg.txt");
//	for ( int x=xx; x<xx+n; x++ )
//	{
//		for ( int y=yy; y<yy+n; y++ )
//		{
//			string item = tz(z,x,y);
//			if ( items.find(item) == items.end() )
//			{
//				neg << item << endl;
//			}
//		}
//	}
//}

Mat mali_img;
Rect mrect;
vector<Rect> rectz;

//
// make rectangles from mouseclicks, dn -> tl, up -> br
//
void onmouse( int event, int x, int y, int d, void *p )
{
	if ( event==1 )
	{
		mrect.x = x;
		mrect.y = y;
		return;
	} 
	if ( event==4 && mrect.x!=0 )
	{
		mrect.width  = x-mrect.x;
		mrect.height = y-mrect.y;
		if ( mrect.width > 0 && mrect.height > 0 ) 
		{
			rectangle(mali_img,mrect,Scalar(0,0,200));
			imshow("mark",mali_img);
			rectz.push_back(mrect);
		    //cerr << event <<" "<< mrect.x <<" "<< mrect.y <<" "<< mrect.width <<" "<< mrect.height <<endl;
		}
		mrect.x = 0;
		mrect.y = 0;
		mrect.width = 0;
		mrect.height = 0;
	}
}

//
// make fixed 64x64 rectangles from mouseclicks, dn -> tl
//
void onmouse2( int event, int x, int y, int d, void *p )
{
	if ( event==1 )
	{
		mrect.x = x;
		mrect.y = y;
		mrect.width  = 64;
		mrect.height = 64;
		rectangle(mali_img,mrect,Scalar(0,0,200));
		imshow("mark",mali_img);
		rectz.push_back(mrect);
	} 
}


void viewrects()
{
	namedWindow("w0",1);
	ifstream mar("mark3.txt");
	while ( mar )
	{
		string pic;
		mar >> pic ;
		if ( pic.empty() )
			continue;
		Mat mali_img = imread(pic);
		if ( mali_img.empty() )
		{
			continue;
		}
		int nrects;
		mar >> nrects;
		if ( nrects<1 )
			break;

		for ( int r=0; r<nrects; r++ )
		{
			int x,y,w,h;
			mar >> x >> y >> w >> h;
			rectangle( mali_img, Rect(x,y,w,h),Scalar(200,0,0) );
		}
		imshow("w0",mali_img);
		int k = waitKey(4000);
		if ( k == 27 ) 
			break;
	}

}


//
// walk a directory, and collect all clicked rects to a positives txt file, mark.txt or such.
//
int main(int argc, char **argv)
{
	int p  = 0;

	//viewrects();
	//return 0;

	namedWindow("mark",1);
	setMouseCallback("mark",onmouse2);
	ofstream pos("pos_c2.txt");
	ofstream neg("neg_c2.txt");
	ofstream mar("mark3.txt",ios::app);
	vector<string> vec=readdir("tilesc/*.jpeg");
	int count = 0;
	for ( size_t i=0; i<vec.size(); i++ )
	{
		count ++;

		std::string uri=std::string("tilesc/") + vec[i];
		mali_img = imread(uri);
		if ( mali_img.empty() )
			continue;

		imshow("mark",mali_img);

		string item = vec[i];
		item.erase( item.find(".jpeg"), item.length() );

		int k = waitKey();
		cerr << int(k) << endl;
		if ( k == 27 )
		{ 
			break;
		}
		else
		if ( k == 8 && i>2 )
		{ 
			i -= 2;
			continue;
		}
		else
		if ( k != ' ' )
		{
			p++;
			cerr << " 1 " << item << " " << i << "\t" << p << "\t" << count << endl;
			pos << item << endl;
		}
		else
		{
			cerr << " 0 " << item << " " << i << "\t" << p << "\t" << count << endl;
			neg << item << endl;
		}

		if ( ! rectz.empty() )
		{
			mar << uri << " " << rectz.size();
			for ( size_t r=0; r<rectz.size(); r++ )
			{
				mar << " " << rectz[r].x << " " << rectz[r].y << " " << rectz[r].width << " " << rectz[r].height ;
			}
			mar << endl;
		}
		rectz.clear();
	}

	////int i  = 0;
	//int z  = 17; //13; 
	//int n  = 30;
	//int xx = 65549;//3932;
	//int yy = 59514;//3787;
	//if ( argc>1) z = atoi(argv[1]);
	//if ( argc>2) xx = atoi(argv[2]);
	//if ( argc>3) yy = atoi(argv[3]);
	//if ( argc>4) n = atoi(argv[4]);
	////makneg(z,xx,yy,n);
	//return 1;
	//for ( int x=xx; x<xx+n; x++ )
	//{
	//	for ( int y=yy; y<yy+n; y++ )
	//	{
	//		i ++;
	//		string item = tz(z,x,y);
	//		cerr << item ;
	//		std::string uri=std::string("tiles/a") + item + std::string(".jpeg");

	//		Mat mali_img = imread(uri);
	//		if ( mali_img.empty() )
	//			continue;
	//		imshow("im",mali_img);
	//		int k = waitKey();
	//		if ( k != ' ' )
	//		{
	//			p++;
	//			cerr << " 1 " << i << "\t" << p << endl;
	//			pos << item << endl;
	//		}
	//		else
	//		{
	//			cerr << " 0 " << i << "\t" << p << endl;
	//			neg << item << endl;
	//		}
	//	}
	//}
}

