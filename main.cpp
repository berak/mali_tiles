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
#include "tools.h"

using namespace std;
using namespace cv;


cv::Mat load(std::string item, std::string folder)
{
	std::string path= folder + std::string("/a") + item + std::string(".jpeg");
	cv::Mat img = imread(path);
	if ( img.empty() )
	{
		IpCapture cap;
		std::string host("ecn.t0.tiles.virtualearth.net");
		std::string fin(".jpeg?g=1145");
		std::string ini("/tiles/a");
		//string host("http://ecn.dynamic.t2.tiles.virtualearth.net");
		//string fin("?mkt=de-de&it=A,G,L&shading=hill&og=2&n=z");
		//std::string ini("/comp/ch/");

		std::string uri = ini + item + fin;
		img = cap.load_jpeg( host.c_str(), uri.c_str() );
		if ( img.empty() )
		{
			std::cerr << "no image : " << uri <<  std::endl;
		} 
		else 
		{
			cv::imwrite( path, img );
			std::cerr << "* ";
		}
	} else	std::cerr << "  ";

	return img;
}


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
// download the tiles for the crowdsource-clicks
// ( i got the .gpx file for that, removed the xml-tags in an editor, left a txt file with: lat lon name )
//
void check_crowd()
{
	int zoom = 17;
	namedWindow("im1",1);
	//ifstream inp("mali-crowd.txt");
	//ifstream inp("missingplaces.txt");
	ifstream inp("mali-crowdsource.txt");
	int count = 0;
	while(!inp.eof())
	{
		double lat,lon;
		inp >> lat;
		inp >> lon;
		int name=-1;
		inp >> name;
		if ( name == -1 )
			break;

		count ++;
		//if ( count < 1700 ) continue;

		int y = lat2tiley(lat,zoom);
		int x = lon2tilex(lon,zoom);
		string item = tz(zoom,x,y);
		Mat img = load(item, "tilesc");
		if ( ! img.empty())
		{
			imshow("im1",img);
		}
		if ( waitKey(1) == 27 )
		{
			return ;
		}

		cerr << format("%4.4f %4.4f  %s  %d",lat,lon,item.c_str(),count) << endl;
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
