#include "tools.h"

double ct(int64 t)
{
	return double(t) / cv::getTickFrequency();
}

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
	std::string quadkey="";
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
	//cerr << zoom << " " << x << " " << y << " " <<  quadkey << endl;
	return quadkey;
}


//#include "ipcap.h"
//
//cv::Mat load(std::string item, std::string folder)
//{
//	std::string path= folder + std::string("/a") + item + std::string(".jpeg");
//	cv::Mat img = imread(path);
//	if ( img.empty() )
//	{
//		IpCapture cap;
//		std::string host("ecn.t0.tiles.virtualearth.net");
//		std::string fin(".jpeg?g=1145");
//		std::string ini("/tiles/a");
//		//string host("http://ecn.dynamic.t2.tiles.virtualearth.net");
//		//string fin("?mkt=de-de&it=A,G,L&shading=hill&og=2&n=z");
//		//std::string ini("/comp/ch/");
//
//		std::string uri = ini + item + fin;
//		img = cap.load_jpeg( host.c_str(), uri.c_str() );
//		if ( img.empty() )
//		{
//			std::cerr << "no image : " << uri <<  std::endl;
//		} 
//		else 
//		{
//			cv::imwrite( path, img );
//			std::cerr << "* ";
//		}
//	} else	std::cerr << "  ";
//
//	return img;
//}
//
