#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "highgui.h"
#include <iostream>

using namespace std; 


int neighbors(const cv::Mat &img, int i, int j) 
/* Return a list of foreground neighboring pixels. */ 
{



}




void ccomp(cv::Mat& img)
{
  CV_Assert(img.depth() == CV_8U);  // accept only uchar images
  CV_Assert(img.channels() == 1);   // just one channel
  //res.create(img.size(),img.type());

  uchar label = 0; 
  const uchar* neighbors[8]; 
  short neighbor_ct; 

  for(int i = 0 ; i < img.rows; i++)
  {
    uchar* p = img.ptr<uchar>(i);







    for(int j = 0; j < img.cols; j++)
    {
      if (p[j] > 0) 
      {
        p[j] = cv::saturate_cast<uchar>( ++label );
        if (label == 255) label = 0; 
      }
    }
  }

}



int main(int argc, const char **argv) {

  if (argc != 3) {
    cerr << "usage: detect image1.jpg image2.jpg";
    return 1; 
  }
  
  /* Morphology */

  int structuring_type = cv::MORPH_ELLIPSE, /* MORPH_{RECT,CROSS,ELLIPSE} */ 
      erosion_factor = 3,
      dilation_factor = 10;
  
  cv::Mat erosion_element = cv::getStructuringElement( structuring_type,
                                                       cv::Size( 2*erosion_factor + 1, 2*erosion_factor+1 ),
                                                       cv::Point( erosion_factor, erosion_factor ) );
  
  cv::Mat dilation_element = cv::getStructuringElement( structuring_type,
                                                        cv::Size( 2*dilation_factor + 1, 2*dilation_factor+1 ),
                                                        cv::Point( dilation_factor, dilation_factor ) );


  cv::Mat img1, img2, delta, thresh, eroded, dilated; 

  img1 = cv::imread( argv[1], CV_LOAD_IMAGE_GRAYSCALE ); 
  img2 = cv::imread( argv[2], CV_LOAD_IMAGE_GRAYSCALE ); 
  
  cv::absdiff(img1, img2, delta); 
  
  // cv::threshold(delta, thresh, 100, 255, CV_THRESH_OTSU); /* Threshold value doesn't matter */ 
  cv::threshold(delta, thresh, 40, 255 , CV_THRESH_BINARY);

  cv::erode( thresh, eroded, erosion_element ); 
  cv::dilate( eroded, dilated, dilation_element ); 


  cv::namedWindow( "Delta", CV_WINDOW_AUTOSIZE ); 

  ccomp(dilated); 
  cv::imshow( "Delta", dilated );   
  cv::waitKey(0); 

  return 0; 

}
