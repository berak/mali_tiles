#include <string>
#include <opencv2/opencv.hpp>

double ct(int64 t);
std::vector<std::string> readdir( const char * dmask );

int lon2tilex(double lon, int z);
int lat2tiley(double lat, int z); 
double tilex2lon(int x, int z) ;
double tiley2lat(int y, int z) ;

std::string tz( int zoom, int x, int y );

//cv::Mat load(std::string item, std::string folder);
