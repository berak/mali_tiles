#ifndef __IpCapture_onboard__
#define __IpCapture_onboard__


#include "opencv/cv.h"

//#include "thread.h"
//#ifdef _WIN32
//	using w32::Thread;
//#endif

class IpCapture 
{
	int sock;
	cv::Mat frame;
	
	const char *readline();
	int readhead();
	
	bool open( const char * host,  const char * uri, int port=80 );
	bool isOpened();
public:

	IpCapture();
	IpCapture( const char * host,  const char * uri, int port=80 );
	~IpCapture();
	
	cv::Mat load_jpeg( const char * host,  const char * uri, int port=80 );
};


#endif // __IpCapture_onboard__
