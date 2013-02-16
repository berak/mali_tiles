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

#include "ipcap.h"
 
using namespace std;
using namespace cv;

// http://ecn.t0.tiles.virtualearth.net/tiles/a03331213223211110.jpeg?g=1146
// http://ecn.t0.tiles.virtualearth.net/tiles/a0333130302322200000.jpeg?g=1146
// http://binged.it/X5EA2n
// http://binged.it/X5EIPq


double ct(int64 t)
{
	return double(t) / cv::getTickFrequency();
}


// arff seems to be the only multi-label format that multiboost likes.
// weka loves it, too.
void write_file( const cv::Mat & features, const cv::Mat & labels, string filename )
{
	bool arff = (filename.find(".arff")<filename.length());
	ofstream of(filename.c_str());
	string sep = " ";
	if ( arff )
	{
		sep = ",";
		of << "%\n% " << features.rows << " rows, " << features.cols << " cols\n%\n\n";
		of << "@relation ne\n\n";
		// we need a column identifier for each one:
		for ( int j=0; j<features.cols; j++ )
		{
			of << ("@attribute _t[");
			of << j;
			of << ("] NUMERIC\n");
		}
		of << ("@attribute class {0,1}\n\n");
		of << ("@data\n");
	}
	for ( int j=0; j<features.rows; j++ )
	{
		for ( int i=0; i<features.cols; i++ )
		{
			of  << (features.at<float>(j,i)) << sep;
		}
		of << int(labels.at<float>(j,0)) << endl;
//		of.flush();
	}		
//	of.close();
}

struct FDetector
{
	cv::Ptr<cv::FeatureDetector>	 detector; 
	cv::Ptr<cv::DescriptorExtractor> extractor;

	FDetector()
	{
		detector  = cv::FeatureDetector::create( "ORB" );
		extractor = cv::DescriptorExtractor::create( "SURF" );
	}

	Mat sift( string item )
	{
		std::string uri=std::string("tiles/a") + item + std::string(".jpeg");
		cv::Mat mali_img = cv::imread(uri);
		if ( mali_img.empty() ) 
			return Mat();

		cv::Mat img;
		cv::cvtColor(mali_img,img,CV_RGB2GRAY);

		vector<cv::KeyPoint> keys;
		detector->detect( img, keys );

		cv::Mat desc;
		extractor->compute( img, keys, desc );

		return desc;
	}

	Mat pushitem( string item, cv::Mat &features, cv::Mat &labels, float label )
	{
		Mat desc = sift(item);
		features.push_back(desc);
		for ( int r=0; r<desc.rows; r++ )
		{
			labels.push_back(label);
		}
		return desc;
	}
};



int main(int argc, char *argv[]) 
{
    cv::initModule_nonfree(); // needed for "SIFT"
	//setWindowProperty("im1", CV_WND_PROP_FULLSCREEN, 1.0);

	cv::Mat features;
	cv::Mat labels;

	bool train=1;
	bool creat=1;

	if ( creat )
	{
		FDetector detect;
		ifstream pos("pos.txt");
		int i = 0;
		while ( ! pos.eof() )
		{
			i++;
			string item;
			pos >> item;
			if ( item.empty() )	break;

			Mat r = detect.pushitem(item,features,labels, 1.0f);
			cout << "1 " << i << " " << r.cols << " " << r.rows << endl;
		}
		int npos = labels.rows;
		cout  << "positives " << npos << endl;

		ifstream neg("neg.txt");
		int mod_n = 1;
		while ( ! neg.eof() )
		{
			i++;
			string item;
			neg >> item;
			if ( item.empty() )
				break;
			if ( i % mod_n != 0 )
				continue;
			detect.pushitem(item,features,labels, 0.0f);
//			cout << "0 " << labels.rows << " " << desc.cols << " " << desc.rows << endl;
		}
		cout  << "negatives " << (labels.rows - npos) << endl;

		//cv::FileStorage sf("sift_s.yml",cv::FileStorage::WRITE);
		//cout << "creat " << labels.rows << ", " << features.rows << endl;
		//sf << "L" << labels;
		//sf << "F" << features;

		write_file( features, labels, "train.arff");
	}

	CvRTrees tree;	

	if ( train )
	{
		int64 t0 = cv::getTickCount();

		cout << "train " << labels.rows << ", " << features.rows <<  ", " << features.cols << endl;

		if ( features.empty() )
		{
			cv::FileStorage sf("sift_s.yml",cv::FileStorage::READ);
			sf["L"] >> labels;
			sf["F"] >> features;
			cout << "read " << labels.rows << ", " << features.rows << endl;
			//write_file( features, labels, "train.arff");
		}

		int64 t1 = cv::getTickCount();
		//cerr << "loaded : " << labels.rows << " " << features.rows << " " << ct(t1-t0) << ".sec" << endl;

		// 
		// train the tree 
		//
		CvRTParams cvrtp;
		cvrtp.max_depth = 25; // this is the main winner
		cvrtp.min_sample_count = 6;
		cvrtp.max_categories = 60;
		cvrtp.term_crit.max_iter = 100;
		
		cerr << cvrtp.term_crit.max_iter << "\tterm_crit.max_iter" << endl;
		cerr << cvrtp.max_depth << "\tmax_depth" << endl;
		cerr << cvrtp.min_sample_count << "\tmin_sample_count" << endl;
		cerr << cvrtp.max_categories << "\tmax_categories" << endl;
		
		tree.train ( features , CV_ROW_SAMPLE , labels,cv::Mat(),cv::Mat(),cv::Mat(),cv::Mat(),cvrtp );
		int64 t2 = cv::getTickCount();
		cout << " train " << ct(t2-t1) << " sec." <<endl;
	
		tree.save("tree_att.yml");
	}


	return 0;
}