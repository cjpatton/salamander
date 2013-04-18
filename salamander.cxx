/* John Muir Institute for the Environment
 * University of California, Davis
 * 
 * salamander.cxx
 * Implementation of the image processing pipeline. This file is part of
 * the Salamander project. 
 * 
 * Copyright (C) 2013 Christopher Patton 
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "salamander.h"
#include "blob.h"
#include "files.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <algorithm> // sort()
#include <cstring>
#include <cstdlib>

void delta( cv::Mat &delta,
            const char *in1, 
            const char *in2, 
            bool thresh, const param_t &options )
/* Subtract a video frame from the previous in the stream and apply
 * binary morphology filter. */
{

  /* Read files from disk */ 
  delta = cv::imread( in1, CV_LOAD_IMAGE_GRAYSCALE ); 
  cv::Mat &A = delta, B = cv::imread( in2, CV_LOAD_IMAGE_GRAYSCALE ); 

  /* Shrink files by factor */ 
  cv::Size size(delta.cols/options.shrink_factor, delta.rows/options.shrink_factor); 
  cv::resize(A, A, size);
  cv::resize(B, B, size);

  /* Pixel-wise absolute difference */  
  cv::absdiff(A, B, delta); 

  if (thresh) {
     //cv::threshold(delta, thresh, 100, 255, CV_THRESH_OTSU); /* Threshold value doesn't matter */ 
     cv::threshold(delta, delta, options.low, 255 , CV_THRESH_BINARY); /* TODO OpenCV's binary threshold 
                                                                          filter is less robust. I might 
                                                                          right my own? */ 
  }
} // original delta() 


void threshold( cv::Mat &img, const char *in, const param_t &options )
/* Binary threshold. */ 
{
  img = cv::imread( in, CV_LOAD_IMAGE_GRAYSCALE ); 
  cv::threshold(img, img, options.low, 255 , CV_THRESH_BINARY); /* TODO */ 
} // threshold()


void read( cv::Mat &img, const char *in, const param_t &options ) 
/* Read and convert an image for the processing pipeline. */
{
  img = cv::imread( in, CV_LOAD_IMAGE_GRAYSCALE ); 

  /* Shrink file by factor */ 
  cv::Size size(img.cols/options.shrink_factor, img.rows/options.shrink_factor); 
  cv::resize(img, img, size);
} // read() 


void delta( cv::Mat &img1, const cv::Mat &img2, 
            bool thresh, const param_t &options )
/* Subtract a video frame from prevoius in stream and apply binary threshold. */
{
  /* Pixel-wise absolute difference */  
  cv::absdiff(img1, img2, img1); 

  if (thresh) {
     cv::threshold(img1, img1, options.low, 255 , CV_THRESH_BINARY); /* TODO */ 
  }
} // delata() 


void delta( cv::Mat &img1, const cv::Mat &img2, const Blob &blob ) 
/* Just calculate delta. */ 
{

  /* Set region of interest */
  cv::Mat A = img1(blob.GetRegion());
  cv::Mat B = img2(blob.GetRegion());
  
  /* Pixel-wise absolute differeince */  
  cv::absdiff(A, B, img1); 

} // delta() 


void threshold( cv::Mat &img, const param_t &options )
/* Apply binary threshold filter to delta. */  
{
  cv::threshold(img, img, options.low, 255 , CV_THRESH_BINARY); /* TODO threshold range */ 
} // threshold() 

void morphology( cv::Mat &img, const param_t &options )
/* Apply binary morphology filter to delta. Erode away weak blobs and dilate 
 * the remaining. */
{

  /* Morphology */

  int structuring_type = cv::MORPH_ELLIPSE; /* MORPH_{RECT,CROSS,ELLIPSE} */ 

  cv::Mat erosion_element = cv::getStructuringElement( structuring_type,
                             cv::Size( 2*options.erode + 1, 2*options.erode+1 ),
                                    cv::Point( options.erode, options.erode ) );

  cv::Mat dilation_element = cv::getStructuringElement( structuring_type,
                          cv::Size( 2*options.dilate + 1, 2*options.dilate+1 ),
                                 cv::Point( options.dilate, options.dilate ) );

  cv::erode( img, img, erosion_element ); 
  cv::dilate( img, img, dilation_element ); 

} // threshold() 

void copy( cv::Mat &dest, const cv::Mat &src )
/* Duplicate an image. */ 
{
  dest = src.clone();
} // copy() 

void write( const cv::Mat &img, const char *out )
/* Cast an image to JPEG compatible output and write to disk. */
{
  cv::imwrite( out, img ); 
} // write() 


void connectedComponents( 
 cv::Mat &im, 
 std::vector<Blob> &sizes ) 
{
  sizes.clear(); 
  ConnectedComponents cc( im ); 
  for (int i = 0; i < cc.size(); i++) 
    sizes.push_back(cc[i]);
} // connectedComponents() 


int getBlobs( const cv::Mat &img, std::vector<Blob> &blobs ) /* TODO */ 
/* Connected component analysis, but give more information about the blobs. 
 * Return a vector of Blob objects with bounding box, centroid, volume (size), 
 * and elongation. */ 
{
  blobs.clear(); 
  ConnectedComponents cc( img ); 
  for (int i = 0; i < cc.size(); i++) 
    blobs.push_back(cc[i]);
  return blobs.size();
} // getBlobs() 

void drawBoundingBox( const char *in, const char *out, const Blob &blob )  /* TODO */ 
/* Draw a bounding box on a JPEG image, as specified by a Blob object. Output
 * to a new file. */ 
{
//  Reader::Pointer reader = Reader::New();
//  reader->SetFileName( in );
//  reader->Update(); 
//  
//  InputPixelType pixel;
//  pixel.SetRed(255); 
//  pixel.SetGreen(50); 
//  pixel.SetBlue(0); 
//
//  InputImageType::IndexType a, b; 
//  typedef itk::LineIterator<InputImageType> IteratorType;
//  IteratorType it(reader->GetOutput(), a, b); 
//
//  /* top */ 
//  a[0] = blob[0]; 
//  b[0] = blob[1]; 
//  a[1] = b[1] = blob[2]; 
//  it = IteratorType(reader->GetOutput(), a, b); 
//  it.GoToBegin(); 
//  while(!it.IsAtEnd()) {
//    it.Set(pixel); 
//    ++it; 
//  }
//  
//  /* left */ 
//  a[0] = b[0] = blob[0]; 
//  a[1] = blob[2]; 
//  b[1] = blob[3]; 
//  it = IteratorType(reader->GetOutput(), a, b); 
//  it.GoToBegin(); 
//  while(!it.IsAtEnd()) {
//    it.Set(pixel); 
//    ++it; 
//  }
//
//  /* right */ 
//  a[0] = b[0] = blob[1]; 
//  a[1] = blob[2]; 
//  b[1] = blob[3]; 
//  it = IteratorType(reader->GetOutput(), a, b); 
//  it.GoToBegin(); 
//  while(!it.IsAtEnd()) {
//    it.Set(pixel); 
//    ++it; 
//  }
//  
//  /* bottom */ 
//  a[0] = blob[0]; 
//  b[0] = blob[1]; 
//  a[1] = b[1] = blob[3]; 
//  it = IteratorType(reader->GetOutput(), a, b); 
//  it.GoToBegin(); 
//  while(!it.IsAtEnd()) {
//    it.Set(pixel); 
//    ++it; 
//  }
//  
//  typedef itk::ImageFileWriter<InputImageType> RGBWriter; 
//  RGBWriter::Pointer writer = RGBWriter::New();
//  writer->SetInput( reader->GetOutput() );
//  writer->SetFileName( out );
//  writer->Update();
} // drawBoundingBox() 
