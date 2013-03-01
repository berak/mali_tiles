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
#include <set>
#include <limits>

 
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

bool valid(float f)
{
	return ( (f>numeric_limits<float>::min()) && (f<numeric_limits<int>::max()) ); // why was the 2nd one int again ?
}

// weka loves it.
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
			switch(features.type())
			{
			case CV_32F: // SIFT,SURF
				{
					float f = (features.at<float>(j,i));
					if (! valid(f)) f = 0.0f;
					of  << f << sep;
				}
				break;
			case CV_8U: // ORB,BRIEF,FREAK
				of << int(features.at<uchar>(j,i)) << sep;
				break;
			default:
				cerr << "invalid data type " << features.type() << endl;
			}
		}
		of << int(labels.at<float>(j,0)) << endl;
	}		
}

string det_name = "MSER";
string ext_name = "SURF";
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

	Mat push( const Mat& desc, Mat &features, Mat &labels, float label )
	{
		if ( desc.empty() )
			return Mat();
		features.push_back(desc);
		for ( int r=0; r<desc.rows; r++ )
		{
			labels.push_back(label);
		}
		return desc;
	}

	Mat extract( string item )
	{
		string uri = string("tiles/a") + item + string(".jpeg");
		Mat mali_img = imread(uri);
		if ( mali_img.empty() ) 
			return Mat();

		return extract( mali_img );
	}

	Mat pushitem( string item, Mat &features, Mat &labels, float label )
	{
		return push( extract(item),features,labels,label );
	}
};



int main(int argc, char *argv[]) 
{
	bool train_tree = false;
	if ( argc>1 ) det_name = argv[1];
	if ( argc>2 ) ext_name = argv[2];
	if ( argc>3 ) neg_ratio = atoi(argv[3]);
	if ( argc>4 ) train_tree = atoi(argv[4]);

	// as an exeption to the rule, Opponent color descriptors require rgb instead of gray
	bool toGray = ext_name[0] !='O' && ext_name[1] !='p';

	cv::initModule_nonfree(); // needed for "SIFT"

	cv::Mat features;
	cv::Mat labels;

	FDetector detect;

	int pos_bummer = 0;
	int pos_descriptors = 0;
	int neg_descriptors = 0;
	map<string,int>  positives;

	ifstream pos("mark3.txt");
	while ( ! pos.eof() )
	{
		string file;
		pos >> file;
		if ( file.empty() )	break;

		positives[file] = 1;

		Mat img = imread(file);
		if ( img.empty() )	break;

		int nr = -1;
		pos >> nr;
		if ( nr < 1 ) break;
		//std::set<Mat> descs;
		int desc_rows_per_pic = 0;
		for ( int r=0; r<nr; r++ )
		{
			int x,y,w,h=-1;
			pos >> x >> y >> w >> h;
			if ( h < 1 ) break;

			if ( x>=191 ) x=191; // err in the marking tool
			if ( y>=191 ) y=191;

			Rect roi(x,y,w,h);

			Mat desc = detect.extract(img(roi),toGray);
			//descs.insert(desc);
			detect.push( desc, features,labels, 1.0f);
			cout << "1 " << r << " " << desc.cols << " " << desc.rows << endl;
			desc_rows_per_pic += desc.rows;
		}
		if ( desc_rows_per_pic == 0 ) pos_bummer += 1;
		pos_descriptors += desc_rows_per_pic;
		if ( pos_descriptors > 20000 )
			break;
	}

	vector<string>fn = readdir("tilesc/*.jpeg");
	for ( size_t n=0,r=0; n<fn.size(); n++ )
	{
		if ( positives.find(fn[n]) != positives.end() )
			continue;
		Mat img = imread(string("tilesc/") + fn[n]);
		if ( img.empty() )	continue;

		Mat desc = detect.extract(img,toGray);
		detect.push( desc, features,labels, 0.0f);
		cout << "0 " << (r++) << " " << desc.cols << " " << desc.rows << endl;
		neg_descriptors += desc.rows;
		if ( neg_descriptors >= pos_descriptors * neg_ratio ) 
			break;
	}
	cerr << "positive bummers " << pos_bummer << endl;
	cerr << "positives " << pos_descriptors << endl;
	cerr << "neg_descriptors " << neg_descriptors << endl;
	write_file( features, labels, format("train_crowd_%s_%s_%d.arff",det_name.c_str(),ext_name.c_str(),neg_ratio) );


	CvRTrees tree;	
	if (train_tree)
	{
		int64 t1 = cv::getTickCount();

		cout << "train " << labels.rows << ", " << features.rows <<  ", " << features.cols << ", " << features.type() <<  endl;

		if ( features.type() != CV_32F )
		{
			Mat f2;
			features.convertTo(f2,CV_32F);
			features = f2;
		}
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
	
		tree.save(format("tree_%s_%s_%d.yml",det_name.c_str(),ext_name.c_str(),neg_ratio).c_str(), "tree");
	}


	return 0;
}
