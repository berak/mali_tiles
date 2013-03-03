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
#include <opencv2/ml/ml.hpp>

#include <fstream>

 
using namespace std;
using namespace cv;

// http://ecn.t0.tiles.virtualearth.net/tiles/a03331213223211110.jpeg?g=1146
// http://ecn.t0.tiles.virtualearth.net/tiles/a0333130302322200000.jpeg?g=1146
// http://binged.it/X5EA2n
// http://binged.it/X5EIPq


#include "tools.h"

string det_name = "PyramidMSER";
string ext_name = "BRIEF";
int neg_ratio = 3;


struct FDetector
{
	cv::Ptr<cv::FeatureDetector>	 detector; 
	cv::Ptr<cv::DescriptorExtractor> extractor;

	FDetector()
	{
		detector  = cv::FeatureDetector::create( det_name );
		extractor = cv::DescriptorExtractor::create( ext_name );
	}

	Mat extract( const Mat& mali_img, bool toGray=true )
	{
		cv::Mat img;
		if ( toGray )
			cv::cvtColor(mali_img,img,CV_RGB2GRAY);
		else
			img = mali_img;

		vector<cv::KeyPoint> keys;
		detector->detect( img, keys );
		if ( keys.empty() )
			return Mat();

		cv::Mat desc;
		extractor->compute( img, keys, desc );

		return desc;
	}

};



int main(int argc, char *argv[]) 
{
//	cv::initModule_nonfree(); // needed for "SIFT"
	size_t nfiles = 0;
	if (argc>1) nfiles=atoi(argv[1]);
	
	int mode = 3;
	if (argc>2) mode=atoi(argv[2]);

	// read the positives:
	map<string,int>  positives;
	ifstream pos("mark3.txt");
	while ( ! pos.eof() )
	{
		string file;
		pos >> file;
		if ( file.empty() )	break;

		int nr = -1;
		pos >> nr;
		if ( nr < 1 ) break;

		positives[file] = nr;

		for ( int r=0; r<nr; r++ )
		{
			int x,y,w,h=-1;
			pos >> x >> y >> w >> h;
		}
	}


	vector<string>fn = readdir("tilesc/*.jpeg");
	if( nfiles > fn.size() ) nfiles = fn.size();

	int confusion[2][2] = {0};

	CvSVM svm;
	CvDTree dtree ;
	CvRTrees rtree;	
	CvBoost boost;	
	switch( mode )
	{
		case 1:
			rtree.load("tree_PyramidMSER_BRIEF_2.yml","tree");
			break;
		case 2:
			dtree.load("dst_PyramidMSER_BRIEF_2.yml","dst");
			break;
		case 3:
			svm.load("svm_PyramidMSER_BRIEF_2.yml","svm");
			break;
		case 4:
			boost.load("boost_PyramidMSER_BRIEF_2.yml","boost");
			break;
	}

	int gpos = 0;
	for ( size_t j=0; j<nfiles; j++ )
	{
		string file = string("tilesc/")+ fn[j];
		Mat img = imread(file);
		if ( img.empty() )	return 1;

		FDetector detect;
		Mat desc = detect.extract(img);
		if ( desc.type() != CV_32F )
		{
			Mat d2;
			desc.convertTo(d2,CV_32F);
			desc = d2;
		}


		int b = (positives.find(file) != positives.end());
		gpos += b;

		float pos = 0;
		for ( int i=0; i<desc.rows; i++ )
		{
			float label=0;
			switch( mode )
			{
				case 1:
					label = rtree.predict( desc.row(i) );
					break;
				case 2:
					{
					CvDTreeNode * prediction = dtree.predict ( desc.row(i) );
					label = prediction -> value ;
					break;
					}
				case 3:
					label = svm.predict( desc.row(i) );
					break;
				case 4:
					label = boost.predict( desc.row(i) );
					break;
			}
			pos += label;
		}
		int a = (pos != 0);
		confusion[a][b] += 1;
		//cerr << format("%s %d %d %3d %3d",file.c_str(),a,b,int(pos),desc.rows) <<   endl;
	}

	cerr << confusion[0][0] << " " << confusion[0][1] << endl;
	cerr << confusion[1][0] << " " << confusion[1][1] << endl;
	int hits = confusion[0][0] + confusion[1][1];
	int miss = confusion[0][1] + confusion[1][0];
	int npos = confusion[1][0] + confusion[1][1];
	int nneg = confusion[0][0] + confusion[0][1];
	int tota = hits + miss;
	float err = float(miss) / float(tota);
	cerr << "hits : " << hits << endl;
	cerr << "miss : " << miss << endl;
	cerr << "npos : " << npos << endl;
	cerr << "nneg : " << nneg << endl;
	cerr << "tota : " << tota << endl;
	cerr << "grou : " << positives.size() << " " << gpos << endl;
	cerr << "err  : " << err << endl;
	return 0;
}
