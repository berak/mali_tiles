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



// 
// download the tiles for the crowdsource-clicks
// ( i get the .gpx file for that, removed the xml-tags in an editor, left a txt file with: lat lon name )
//
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



//
// download tiles for raster, given position zoom and patchcount
//
void download_raster(int z, int xx, int yy, int n )
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


	//int z  = 17; //13; 
	//int n  = 30;
	//int xx = 65549;//3932;
	//int yy = 59514;//3787;
	//if ( argc>1) z = atoi(argv[1]);
	//if ( argc>2) xx = atoi(argv[2]);
	//if ( argc>3) yy = atoi(argv[3]);
	//if ( argc>4) n = atoi(argv[4]);
	//download_raster(z,xx,yy,n);
	//return 0;

	return 0;
}
