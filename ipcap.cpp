#include "opencv/highgui.h"
//using namespace cv;

//#include <conio.h>
#include <fstream>

#include "birds.h"
//#include "thread.h"
#include "ipcap.h"

//#ifdef _WIN32
//	struct CriticalSection : public CRITICAL_SECTION 
//	{
//		CriticalSection()	{ InitializeCriticalSection(this); 	}	
//		~CriticalSection() 	{ DeleteCriticalSection (this);    	}
//
//		void lock()			{ EnterCriticalSection( this );		}
//		void unlock()		{ LeaveCriticalSection( this );		}
//	} g_critter;
//#else
//	Mutex g_critter;
//#endif


IpCapture::IpCapture()
	: sock(-1)
{
}


IpCapture::IpCapture( const char * host,  const char * uri, int port )
{
	load_jpeg(host,uri,port);
}


IpCapture::~IpCapture()
{
	if ( sock > -1 )
		Birds::Close(sock);
}


bool IpCapture::isOpened()
{
	return sock > -1;
}



bool IpCapture::open( const char * host,  const char * uri, int port )
{
	if ( isOpened() )
	{
		Birds::Close(sock);
		sock = -1;
		frame = cv::Mat();
	}

	sock = Birds::Client( host, port );
	if ( sock < 0 ) return false;

	char req[200];
	sprintf(req,"GET %s HTTP/1.1\r\nHOST:%s\r\nAccept:*;\r\n\r\n", uri, host );
	//std::cerr << req;
	int r = Birds::Write(sock, req,0);
	return true;
}

cv::Mat IpCapture::load_jpeg( const char * host,  const char * uri, int port )
{
	if ( IpCapture::open(host,uri,port) )
	{
		// make a copy of uri and clean of start '/' as of '?' and trailing
		char scpy[400];
		int sstart=0;
		if ( uri[0]=='/')
			sstart=1;
		strcpy(scpy,uri+sstart);
		char * ask = strchr(scpy,'?');
		if ( ask != 0 )
		{
			*ask = 0;
		}
		readhead();

		int finished = 0;
		std::cerr << scpy << std::endl;
		//std::ofstream img(scpy, std::ios_base::binary );
		// read bytes until the jpeg end sequence ff d9 is hit
		std::vector<char> bytes;
		while ( true )
		{
			char c = Birds::ReadByte(sock);

			//img << c;
			bytes.push_back(c);

			if ( c == char(0xff) ) 	finished = 1;
			else if ( c == char(0xd9) && finished == 1 ) 	break;
			else finished = 0;

			const char * err = Birds::Error();
			if ( err && err[0] )
			{
				printf("%s\n", err );
				return cv::Mat();
			}
		}
		//img.close();
		return cv::imdecode(bytes,1);
	}
	return cv::Mat();
}

const char* IpCapture::readline()
{
	static char line[512];
	int n = 0;
	line[n] = 0;
	while( isOpened() && (n<511) )
	{
		char c = Birds::ReadByte(sock);
		if ( c == '\r' )
			continue;
		if ( c == '\n' )
			break;
		line[n] = c;
		n ++;
		//std::cerr << c;
	}
	line[n] = 0;
	return line;
}


int IpCapture::readhead()
{
	int nbytes = -1;
	int nlines = 0;
	while( isOpened() )
	{
		const char *l = readline();
		if ( nbytes < 0 )
			sscanf(l, "Content-Length: %i",&nbytes );
		if ( strlen(l) == 0  && nlines > 1 )
			break;
		nlines ++;
	}
	return nbytes;
}

//void IpCapture::run()
//{
//	int frame_id = 0;
//	readhead();
//	while( isOpened() )
//	{
//		int nb = readhead();
//		//if ( nb < 1 ) nb = 0xffff;
//		int finished = 0;
//		int t = 0;
//		std::vector<char> bytes;
//		// read bytes until the jpeg end sequence ff d9 is hit
//		while ( true )
//		{
//			char c = Birds::ReadByte(sock);
//
//			if ( c == char(0xff) ) 	finished = 1;
//			else if ( c == char(0xd9) && finished == 1 ) 	break;
//			else finished = 0;
//
//			bytes.push_back(c);
//			//if ( (bytes.size() >= nb) )	break;
//
//			const char * err = Birds::Error();
//			if ( err && err[0] )
//			{
//				printf("%s\n", err );
//				return;
//			}
//		}
//
//		g_critter.lock();
//		frame = cv::imdecode(bytes,1);
//		g_critter.unlock();
//
//		printf( "%2d %6d\n", frame_id, bytes.size() );
//		frame_id ++;
//	}
//}
//
////unsigned char * IpCapture::lockPixels()
////{
////	g_critter.lock();
////	return frame.data;
////}
////void IpCapture::unlockPixels()
////{
////	g_critter.unlock();
////}
//
//
//IpCapture & operator >> ( IpCapture & cap, cv::Mat & mat )
//{
//	g_critter.lock();
//		mat = cap.frame.clone();
//	g_critter.unlock();
//	return cap;
//}
//
//
//
//
