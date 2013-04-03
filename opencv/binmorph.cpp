#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "highgui.h"
#include <iostream> 

using namespace std;



int main(int argc, const char **argv) {

  if (argc != 3) {
    cerr << "usage: binmorph image1.jpg image2.jpg";
    return 1; 
  }

  /* Morphology */

  int structuring_type = cv::MORPH_ELLIPSE, /* MORPH_{RECT,CROSS,ELLIPSE} */ 
      dilation_factor = 15,
      erosion_factor = 1;  
  
  cv::Mat erosion_element = cv::getStructuringElement( structuring_type,
                                                       cv::Size( 2*erosion_factor + 1, 2*erosion_factor+1 ),
                                                       cv::Point( erosion_factor, erosion_factor ) );
  
  cv::Mat dilation_element = cv::getStructuringElement( structuring_type,
                                                        cv::Size( 2*dilation_factor + 1, 2*dilation_factor+1 ),
                                                        cv::Point( dilation_factor, dilation_factor ) );

  cv::Mat img1, img2, delta, eroded, dilated;

  img1 = cv::imread( argv[1] ); 
  img2 = cv::imread( argv[2] );

  cv::absdiff(img1, img2, delta);
  cv::erode( delta, eroded, erosion_element ); 
  cv::dilate( eroded, dilated, dilation_element ); 
  
  cv::namedWindow( "Delta", CV_WINDOW_AUTOSIZE ); 

  cv::imshow( "Delta", dilated ); 
  cv::waitKey(0); 


  return 0; 

}
