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

  cv::Mat img1, img2, delta, thresh;

  img1 = cv::imread( argv[1], CV_LOAD_IMAGE_GRAYSCALE ); 
  img2 = cv::imread( argv[2], CV_LOAD_IMAGE_GRAYSCALE ); 
  
  cv::absdiff(img1, img2, delta); 
  // cv::threshold(delta, thresh, 100, 255, CV_THRESH_OTSU); /* Threshold value doesn't matter */ 
  cv::threshold(delta, thresh, 40, 255 , CV_THRESH_BINARY);
  
  cv::namedWindow( "Delta", CV_WINDOW_AUTOSIZE ); 
  
  cv::imshow( "Delta", thresh ); 

  cv::waitKey(0); 


  return 0; 

}
