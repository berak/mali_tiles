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



void makneg(int z, int xx, int yy, int n )
{
	ifstream pos("pos.txt");
	map<string,int> items;
	while( ! pos.eof() )
	{
		string s;
		pos >> s;
		if ( s.empty() )
			break;
		items[s] = 1;
	}
	ofstream neg("neg.txt");
	for ( int x=xx; x<xx+n; x++ )
	{
		for ( int y=yy; y<yy+n; y++ )
		{
			string item = tz(z,x,y);
			if ( items.find(item) == items.end() )
			{
				neg << item << endl;
			}
		}
	}
}

int main(int argc, char **argv)
{
	int p  = 0;
	//int i  = 0;
	int z  = 17; //13; 
	int n  = 30;
	int xx = 65549;//3932;
	int yy = 59514;//3787;
	if ( argc>1) z = atoi(argv[1]);
	if ( argc>2) xx = atoi(argv[2]);
	if ( argc>3) yy = atoi(argv[3]);
	if ( argc>4) n = atoi(argv[4]);
	//makneg(z,xx,yy,n);
	//return 1;


	namedWindow("mark",1);
	ofstream pos("pos_c.txt");
	ofstream neg("neg_c.txt");
	vector<string> vec=readdir("tilesc/*.jpeg");
	for ( size_t i=0; i<vec.size(); i++ )
	{
		std::string uri=std::string("tilesc/") + vec[i];

		Mat mali_img = imread(uri);
		if ( mali_img.empty() )
			continue;
		imshow("mark",mali_img);
		string item = vec[i];
		item.erase( item.find(".jpeg"), item.length() );
		int k = waitKey();
		if ( k != ' ' )
		{
			p++;
			cerr << " 1 " << i << "\t" << p << endl;
			pos << item << endl;
		}
		else
		{
			cerr << " 0 " << i << "\t" << p << endl;
			neg << item << endl;
		}
	}
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

