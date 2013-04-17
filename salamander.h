/* John Muir Institute for the Environment
 * University of California, Davis
 * 
 * salamander.h
 * Header for ITK includes and type declarations. Declarations for 
 * functions that implement image processing pipelines. This file is 
 * part of the Salamander project. 
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


#ifndef SALAMANDER_H
#define SALAMANDER_H

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "highgui.h"

#include <fstream>
#include <vector>
#include <string>


/* Forward declarations */ 
class Blob; 
struct param_t; 
 
void delta( cv::Mat&,
            const char *, 
            const char *, 
            bool thresh, const param_t& );

void threshold( cv::Mat&, const char *, const param_t &options );

void read( cv::Mat&, const char *, const param_t &options ); 

void delta( cv::Mat&, const cv::Mat&, 
            bool thresh, const param_t &options );

void delta( cv::Mat&, const cv::Mat&, const Blob& ); 
                          
void threshold( cv::Mat&, const param_t &options ); 

void morphology( cv::Mat&, const param_t &options ); 

void write( const cv::Mat&, const char * ); 

void copy( cv::Mat&, const cv::Mat& ); 

void connectedComponents( 
  cv::Mat &im, 
  std::vector<Blob> &sizes );

int getBlobs( const cv::Mat &, std::vector<Blob> &blobs ); 

void drawBoundingBox( const char *in, const char *out, const Blob &blob );

#endif
