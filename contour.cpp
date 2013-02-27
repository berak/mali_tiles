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
 
using namespace std;
using namespace cv;

//
// 17 64443 59342 40


// http://ecn.t0.tiles.virtualearth.net/tiles/a03331213223211110.jpeg?g=1146
// http://ecn.t0.tiles.virtualearth.net/tiles/a0333130302322200000.jpeg?g=1146
// http://binged.it/X5EA2n
// http://binged.it/X5EIPq



static int thresh = 4000; //1000
static int N = 32; //16
static int NUM_SHARPEN = 5;
static double angle( Point pt1, Point pt2, Point pt0 )
{
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

void findSquares(Mat& image, vector<vector<Point> >& squares )
{
    squares.clear();

    Mat pyr, timg, gray0, gray;

    if(image.empty()) return;

    gray0 = Mat(image.size(), CV_8U);
    // down-scale and upscale the image to filter out the noise
    //pyrDown(image, pyr, Size(image.cols/2, image.rows/2));
    //pyrUp(pyr, timg, image.size());
    //Sharpen image, because they are really blurry already
    GaussianBlur(image, pyr, cv::Size(0, 0), 3);
    addWeighted(image, 1.25, pyr, -0.25, 0, timg);
    for(int m=0; m<NUM_SHARPEN; ++m)
        addWeighted(timg, 1.25, pyr, -0.25, 0, timg);
    //timg = image.clone();
    vector<vector<Point> > contours;

    // find squares in every color plane of the image
    for( int c = 0; c < 3; c++ )
    {
        int ch[] = {c, 0};
        mixChannels(&timg, 1, &gray0, 1, ch, 1);

        // try several threshold levels
        for( int l = 1; l < N; l++ )
        {
            // hack: use Canny instead of zero threshold level.
            // Canny helps to catch squares with gradient shading
#if USE_CANNY
            if( l == 0 )
            {
                // apply Canny. Take the upper threshold from slider
                // and set the lower to 0 (which forces edges merging)
                Canny(gray0, gray, 0, thresh, 5);
                // dilate canny output to remove potential
                // holes between edge segments
                dilate(gray, gray, Mat(), Point(-1,-1));
            }
            else
            {
#endif
                // apply threshold if l!=0:
                // tgray(x,y) = gray(x,y) < (l+1)*255/N ? 255 : 0
                gray = gray0 >= (l+1)*255/N;
#if USE_CANNY
            }
#endif
            // find contours and store them all as a list
            findContours(gray, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

            vector<Point> approx;

            // test each contour
            for( size_t i = 0; i < contours.size(); i++ )
            {
                // approximate contour with accuracy proportional
                // to the contour perimeter
                approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);

                // square contours should have 4 vertices after approximation
                // relatively large area (to filter out noisy contours)
                // and be convex.
                // Note: absolute value of an area is used because
                // area may be positive or negative - in accordance with the
                // contour orientation
                int area = std::abs(contourArea(Mat(approx)));
                int count = approx.size();
                if( count >= 4 && !(count & 1) &&
                    area >= 64 && area <= 6400 ) //&& isContourConvex(Mat(approx))
                {
                    double maxCosine = 0;

                    /*
for( int j = 2; j < 5; j++ )
{
// find the maximum cosine of the angle between joint edges
double cosine = fabs(angle(approx[j%4], approx[j-2], approx[j-1]));
maxCosine = MAX(maxCosine, cosine);
}
*/
                    for(int j = 0; j < count; ++j){
                        int k = j-1;
                        int m = j+1;
                        if(j == 0) k = count-1;
                        else if(j == (count-1)) m = 0;
                        double cosine = std::abs(angle(approx[k], approx[m], approx[j]));
                        maxCosine = std::max(maxCosine, cosine);
                    }

                    // if cosines of all angles are small
                    // (all angles are ~90 degree) then write quandrange
                    // vertices to resultant sequence
                    if( maxCosine < 0.3)
                        squares.push_back(approx);
                }
            }
        }
    }
}



int main(int argc, char *argv[]) 
{
	RNG rng;
	std::string path ="tiles/a03331123332113322.jpeg";
	//std::string path ="tiles1/a033313120003033222.jpeg";
	if ( argc>1 ) 
	{
		path = argv[1];
	}

	cv::Mat mali_img = cv::imread(path);


	vector<vector<Point> > squares;
	findSquares( mali_img, squares );
	namedWindow("w0");
	for (  size_t i=0; i<squares.size(); i++ )
	{
		Scalar color = Scalar( 0,0,255);//rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
		const cv::Point * pc = &(squares[i][0]);
		int nc = squares[i].size();
		cv::polylines( mali_img, &pc, &nc, 1, 0, color );
		for ( size_t j=0; j<squares[i].size(); j++ )
		{
			cv::circle(mali_img,squares[i][j],2,Scalar(0,255,0));
			cerr << squares[i][j] << " ";
		}
		cerr << endl;
	}
	cerr << squares.size() << " found in " << path << endl;
	imshow("w0",mali_img);
	waitKey(0);

return 0;

	cv::Mat img = mali_img;
	cv::Mat rez;
	resize(mali_img, rez,Size(mali_img.rows*2,mali_img.cols*2),0,0,INTER_CUBIC);

	Mat txx;
    cv::cvtColor(rez,img,CV_RGB2GRAY);

	//equalizeHist(img,img);
//	bilateralFilter(txx,img,3, 30, 110);

	Mat canny_output;
	vector<vector<Point> > contours, good;

	vector<Vec4i> hierarchy;

	/// Detect edges using canny
	int thresh = 80;
	Canny( img, canny_output, thresh, thresh*2, 3 );
	//Canny( img, img, thresh, thresh*2, 3 );
	//threshold(img,canny_output,120,255,4);
	/// Find contours  
	findContours( canny_output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_TC89_KCOS, Point(0, 0) );

	/// Draw contours

	cv::namedWindow("im0",1);
	cv::namedWindow("im1",1);
	cv::namedWindow("im2",1);
	imshow("im0", mali_img);
	imshow("im1", canny_output);

	for( int i = 0; i< contours.size(); i++ )
	{ 
		Mat draw = rez.clone();
		Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
		drawContours( draw, contours, i, color, 1, 8, hierarchy, 0, Point() ); 
		int k = waitKey();
		if ( k != ' ' )
		{
			cerr << i << endl;
			good.push_back(contours[i]);
		}
		imshow("im2", draw);
		//Mat contour;
		//cv::approxPolyDP(cv::Mat(contours[i]), contour, 3, true);
		//const cv::Point * pc = contour.ptr<cv::Point>(0);
		//int nc = contour.rows;
		//cv::polylines( mali_img, &pc, &nc, 1, 0, color );
	}
	//cerr << good.size() << endl;
	//cv::FileStorage fs(cv::format("contours.yml",item.c_str()),cv::FileStorage::WRITE);
	//for( int i = 0; i< good.size(); i++ )
	//{ 
	//	fs << format("K%d",i) << good[i];
	//}

	//cv::FileStorage fs(cv::format("contours.yml",item.c_str()),cv::FileStorage::READ);
	//for( int i = 0; i<1000; i++ )
	//{ 
	//	vector < Point > v;
	//	fs[format("K%d",i)] >> v;
	//	if ( v.empty() )
	//		break;
	//	good.push_back(v);
	//}

	for( int i = 0; i<contours.size(); i++ )
	{ 
		for ( int j=0; j<good.size(); j++ )
		{
			double d = matchShapes( contours[i], good[j],CV_CONTOURS_MATCH_I2, 0 );
			//cerr << i << " " << j << " " << d << endl;
			if ( d < 0.5 )
			{
				Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
				const cv::Point * pc = &(good[j][0]);
				int nc = good[j].size();
				//const cv::Point * pc = &(contours[i][0]);
				//int nc = contours[i].size();
				cv::polylines( mali_img, &pc, &nc, 1, 0, color );
			}
		}
	}

	waitKey(0);
	return 0;
}
