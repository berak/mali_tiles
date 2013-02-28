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
	return ( (f>numeric_limits<float>::min()) && (f<numeric_limits<int>::max()) );
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
			float f = (features.at<float>(j,i));
			if (! valid(f)) f = 0.0f;
			of  << f << sep;
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

	Mat extract( const Mat& mali_img )
	{
		cv::Mat img;
		cv::cvtColor(mali_img,img,CV_RGB2GRAY);

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
	if ( argc>1 ) det_name = argv[1];
	if ( argc>2 ) ext_name = argv[2];
	if ( argc>3 ) neg_ratio = atoi(argv[3]);

    cv::initModule_nonfree(); // needed for "SIFT"

	cv::Mat features;
	cv::Mat labels;

	FDetector detect;

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
		for ( int r=0; r<nr; r++ )
		{
			int x,y,w,h=-1;
			pos >> x >> y >> w >> h;
			if ( h < 1 ) break;

			if ( x>=191 ) x=191; // err in the marking tool
			if ( y>=191 ) y=191;

			Rect roi(x,y,w,h);

			Mat desc = detect.extract(img(roi));
			//descs.insert(desc);
			detect.push( desc, features,labels, 1.0f);
			cout << "1 " << r << " " << desc.cols << " " << desc.rows << endl;
			pos_descriptors += desc.rows;
		}
		if ( pos_descriptors > 20000 )
			break;
	}

	cerr << "positives " << pos_descriptors << endl;
	vector<string>fn = readdir("tilesc/*.jpeg");
	for ( size_t n=0,r=0; n<fn.size(); n++ )
	{
		if ( positives.find(fn[n]) != positives.end() )
			continue;
		Mat img = imread(string("tilesc/") + fn[n]);
		if ( img.empty() )	continue;

		Mat desc = detect.extract(img);
		detect.push( desc, features,labels, 0.0f);
		cout << "0 " << (r++) << " " << desc.cols << " " << desc.rows << endl;
		neg_descriptors += desc.rows;
		if ( neg_descriptors >= pos_descriptors * neg_ratio ) 
			break;
	}
	cerr << "neg_descriptors " << neg_descriptors << endl;
	write_file( features, labels, format("train_crowd_%s_%s_%d.arff",det_name.c_str(),ext_name.c_str(),neg_ratio) );

	return 1;

	//bool train = 0;
	//bool creat = 1;
	//int mod_n  = 1;
	//if ( argc>1 ) mod_n = atoi(argv[1]);

	//if ( creat )
	//{
	//	FDetector detect;
	//	ifstream pos("pos.txt");
	//	int i = 0;
	//	while ( ! pos.eof() )
	//	{
	//		i++;
	//		string item;
	//		pos >> item;
	//		if ( item.empty() )	break;

	//		Mat r = detect.pushitem(item,features,labels, 1.0f);
	//		cout << "1 " << i << " " << r.cols << " " << r.rows << endl;
	//	}
	//	int npos = labels.rows;

	//	ifstream neg("neg.txt");
	//	while ( ! neg.eof() )
	//	{
	//		i++;
	//		string item;
	//		neg >> item;
	//		if ( item.empty() )
	//			break;
	//		if ( i % mod_n != 0 )
	//			continue;
	//		Mat r = detect.pushitem(item,features,labels, 0.0f);
	//		cout << "0 " << labels.rows << " " << r.cols << " " << r.rows << endl;
	//	}
	//	cout  << "positives " << npos << endl;
	//	cout  << "negatives " << (labels.rows - npos) << endl;

	//	//cv::FileStorage sf("sift_s.yml",cv::FileStorage::WRITE);
	//	//cout << "creat " << labels.rows << ", " << features.rows << endl;
	//	//sf << "L" << labels;
	//	//sf << "F" << features;

	//	write_file( features, labels, "train.arff");
	//}

	//CvRTrees tree;	

	//if ( train )
	//{
	//	int64 t0 = cv::getTickCount();

	//	cout << "train " << labels.rows << ", " << features.rows <<  ", " << features.cols << endl;

	//	if ( features.empty() )
	//	{
	//		cv::FileStorage sf("sift_s.yml",cv::FileStorage::READ);
	//		sf["L"] >> labels;
	//		sf["F"] >> features;
	//		cout << "read " << labels.rows << ", " << features.rows << endl;
	//		//write_file( features, labels, "train.arff");
	//	}

	//	int64 t1 = cv::getTickCount();
	//	//cerr << "loaded : " << labels.rows << " " << features.rows << " " << ct(t1-t0) << ".sec" << endl;

	//	// 
	//	// train the tree 
	//	//
	//	CvRTParams cvrtp;
	//	cvrtp.max_depth = 25; // this is the main winner
	//	cvrtp.min_sample_count = 6;
	//	cvrtp.max_categories = 60;
	//	cvrtp.term_crit.max_iter = 100;
	//	
	//	cerr << cvrtp.term_crit.max_iter << "\tterm_crit.max_iter" << endl;
	//	cerr << cvrtp.max_depth << "\tmax_depth" << endl;
	//	cerr << cvrtp.min_sample_count << "\tmin_sample_count" << endl;
	//	cerr << cvrtp.max_categories << "\tmax_categories" << endl;
	//	
	//	tree.train ( features , CV_ROW_SAMPLE , labels,cv::Mat(),cv::Mat(),cv::Mat(),cv::Mat(),cvrtp );
	//	int64 t2 = cv::getTickCount();
	//	cout << " train " << ct(t2-t1) << " sec." <<endl;
	//
	//	tree.save("tree.yml");
	//}


	return 0;
}
