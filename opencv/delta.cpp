#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "highgui.h"
#include <iostream>

using namespace std; 



int main(int argc, const char **argv) {

  if (argc != 3) {
    cerr << "usage: delta image1.jpg image2.jpg";
    return 1; 
  }

  cv::Mat img1, img2, delta;

  img1 = cv::imread( argv[1] ); 
  img2 = cv::imread( argv[2] );


  cv::absdiff(img1, img2, delta); 
  
  cv::namedWindow( "Delta", CV_WINDOW_AUTOSIZE ); 
  
  cv::imshow( "Delta", delta ); 

  cv::waitKey(0); 


  return 0; 

}
