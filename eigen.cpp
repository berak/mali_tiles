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

typedef vector<Mat> MatVec;
typedef vector<int> IntVec;
typedef vector<string> StrVec;

struct EigenHeim
{
	Ptr<FaceRecognizer> model;
	string name;

	EigenHeim( string kind )
	{
		name=kind;
		if ( kind == "fisher" ) {
			model = createFisherFaceRecognizer();
		} else if ( kind == "eigen"  ) {
			model = createEigenFaceRecognizer();
		} else if ( kind == "lbp"  ) {
			model = createLBPHFaceRecognizer();
		} else {
			name="none";
		}
	}

	Mat load( string item )
	{
		std::string uri = std::string("tiles/a") + item + std::string(".jpeg");
		cv::Mat m1 = cv::imread(uri, 0);
		if ( m1.empty() ) 
			return Mat();

		Mat m2;
		//cv::cvtColor(m1,m2,CV_RGB2GRAY);
		cv::equalizeHist( m1, m2 );

		if ( name=="lbp" ) 
			return m2;
		return m2.reshape(1,1);
	}


	void read_train(const string &fn, int label, MatVec & images, IntVec & labels, int maxiter)
	{
		ifstream pos(fn.c_str());
		int i = 0;
		while ( ! pos.eof() )
		{
			string item;
			pos >> item;
			if ( item.empty() ) 	
				break;

			i++;
			if ( i % 2 != 0 )		
				continue;
			if ( i/2 > maxiter )		
				break;

			add_train(item,label,images,labels);
		}
	}

	void read_test(const string & fn, int label, IntVec & labels, StrVec & items, int maxiter)
	{
		ifstream pos(fn.c_str());
		int i = 0;
		while ( ! pos.eof() )
		{
			string item;
			pos >> item;
			if ( item.empty() ) 	break;

			i++;
			if ( i % 2 == 0 )		
				continue;
			if ( i/2 > maxiter )		
				break;

			labels.push_back(label);
			items.push_back(item);
		}
	}

	void add_train( string item, int label, MatVec & images, IntVec & labels )
	{
		Mat img = load(item);
		if ( img.empty() ) return ;

		images.push_back(img);
        labels.push_back(label);
	}

	void train(const MatVec & images, const IntVec & labels, bool save)
	{
		int64 t1 = cv::getTickCount();

		model->train(images, labels);
		int64 t2 = cv::getTickCount();
		cerr << name << " trained " << ct(t2-t1) << " sec." << endl;
		if ( save )
		{
			model->save( format("eigen_%s.yml",name.c_str()) );
			int64 t3 = cv::getTickCount();
			cerr << name << " saved  " << ct(t3-t2) << " sec." << endl;
		}
	}

	int test(string item)
	{
		Mat image=load(item);
		if ( image.empty() ) return -1;
		double dist = 0;
		int predicted = 0;
		model->predict(image,predicted,dist);
		return predicted;
	}
};



int main(int argc, char *argv[]) 
{
    //cv::initModule_nonfree(); // needed for "SIFT"
	//setWindowProperty("im1", CV_WND_PROP_FULLSCREEN, 1.0);

	IntVec labels;
	StrVec items;

	string kind = "lbp";
	if ( argc>1 ) kind = argv[1];

	EigenHeim detect(kind);
	//EigenHeim detect("fisher");
	//EigenHeim detect("eigen");


	size_t maxiters = 400;
	if ( argc>2 ) maxiters = atoi(argv[2]);

	{	// inner scope to release the images early
		MatVec images;
		detect.read_train("pos.txt",1,images,labels,maxiters);
		int npos = labels.size();
		detect.read_train("neg.txt",0,images,labels,maxiters);

		cerr << detect.name  << " positives " << npos << endl;
		cerr << detect.name  << " negatives " << (labels.size() - npos) << endl;

		detect.train(images,labels,false);
	}

	
	int64 t1 = cv::getTickCount();
	//cerr << "loaded : " << labels.rows << " " << features.rows << " " << ct(t1-t0) << ".sec" << endl;


	labels.clear();
	items.clear();
	detect.read_test("pos.txt",1,labels,items,maxiters);
	int npos = labels.size();
	cerr << detect.name << " positives " << npos << endl;
	detect.read_test("neg.txt",0,labels,items,maxiters);
	cerr << detect.name  << " negatives " << (labels.size() - npos) << endl;
	int64 t2 = cv::getTickCount();
	cerr << detect.name << " create " << items.size() << " items, " << ct(t2-t1) << " sec." << endl;

	int good = 0;
	int MAX_IT = min(items.size(), maxiters);
	int confusion[2][2] = {0};
	for ( int i=0; i<MAX_IT; i++ )
	{
		int id = i; //rng.next() % MAX_IT;
		string item = items[id];
		int ground = labels[id];
		int predic = detect.test(item);
		int ok = (predic==ground);
		confusion[predic][ground] += 1;
		//if ( ! ok )	cerr << item << " " << predic << " " << ground << " " << ok << endl; 
		good += (ok);
		if ( i%100==0 ) cerr << ".";
	}
	cerr << detect.name << " maxiters " << MAX_IT << endl;
	int64 t3 = cv::getTickCount();
	cerr << detect.name << " confusion:" << endl;
	cerr << detect.name << " " << confusion[0][0] << "\t" << confusion[0][1] << endl;
	cerr << detect.name << " " <<  confusion[1][0] << "\t" << confusion[1][1] << endl;
	cerr << detect.name << " test: " << good << " " << (MAX_IT-good) << " " << (float(good)/float(MAX_IT)) << " " << ct(t3-t2) << " sec." << endl;
	return 0;
}
