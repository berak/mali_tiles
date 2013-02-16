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
	return quadkey;
	cerr << zoom << " " << x << " " << y << " " <<  quadkey << endl;
}


void findPositives()
{
	int good = 0;
	int cnt = 0;
	ifstream in("pos_.txt");
	ofstream posi("posi.txt");
	ofstream nega("nega.txt");
	while(!in.eof())
	{
		string it;
		in >> it;
		int p;
		in >> p;
		good += p;
		cnt ++;
		//cerr << it << " " << p << " " << good << " " << cnt << endl;

		if ( p != 0 )
		{
			posi << it << endl;
		}
		else
		{
			nega << it << endl;
		}
	}

	cerr << good << " / " << cnt << endl;
}

int main()
{
	int p  = 0;
	int i  = 0;
	int z  = 18;
	int n  = 100;
	int xx = 129100;
	int yy = 119900;
	namedWindow("im",0);
	ofstream pos("pos2.txt");
	for ( int x=xx; x<xx+n; x++ )
	{
		for ( int y=yy; y<yy+n; y++ )
		{
			i ++;
			if ( i <  7533 ) continue;

			string item = tz(z,x,y);
			cerr << item ;
			std::string uri=std::string("tiles/a") + item + std::string(".jpeg");

			Mat mali_img = imread(uri);
			imshow("im",mali_img);
			int k = waitKey();
			if ( k == '?' )
			{
				i--;
				y--;
				continue;
			}
			if ( k != ' ' )
			{
				p++;
				cerr << " 1 " << i << "\t" << p << endl;
				pos << item << " 1" << endl;
			}
			else
			{
				cerr << " 0 " << i << "\t" << p << endl;
				pos << item << " 0" << endl;
			}
		}
	}
}

