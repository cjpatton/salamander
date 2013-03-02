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

#include "itkImage.h"
#include "itkRGBPixel.h"
#include "itkJPEGImageIO.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIterator.h"
#include "itkSubtractImageFilter.h"
#include "itkAbsImageFilter.h"
#include "itkStatisticsImageFilter.h"
#include "itkRGBToLuminanceImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryErodeImageFilter.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkRelabelComponentImageFilter.h"
#include "itkBinaryImageToLabelMapFilter.h"
#include "itkLabelMapToLabelImageFilter.h"
#include "itkLabelGeometryImageFilter.h"
#include "itkShrinkImageFilter.h"
#include "itkImageDuplicator.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkAddImageFilter.h"
#include "itkLineIterator.h"


#include <fstream>
#include <vector>
#include <string>


/**
 * Types for the various ITK pipelines. 
 */
const unsigned int      Dim = 2; 
typedef itk::RGBPixel<unsigned char>    InputPixelType;
typedef itk::Image<InputPixelType, Dim> InputImageType;
typedef itk::Image<float, Dim>   ImageType;
typedef itk::Image<unsigned char, Dim>  OutputImageType;

typedef itk::ImageFileReader<InputImageType>  Reader;
typedef itk::ImageFileWriter<OutputImageType> Writer; 
typedef itk::JPEGImageIO                ImageOfType; 

typedef itk::ImageDuplicator<ImageType> DuplicatorType;

/* Subtraction and binary thresholding */
typedef itk::RGBToLuminanceImageFilter<InputImageType, ImageType> 
    ConverterType;

typedef itk::SubtractImageFilter<ImageType, ImageType, ImageType> 
    SubtractImageFilterType;

typedef itk::ShrinkImageFilter<ImageType, ImageType> 
    ShrinkImageFilterType; 

typedef itk::RegionOfInterestImageFilter<ImageType, ImageType> 
    ROIFilterType; 

typedef itk::AbsImageFilter<ImageType, ImageType> 
    AbsImageFilterType; 

typedef itk::BinaryThresholdImageFilter<ImageType, ImageType>
    BinaryThresholdImageFilterType;
const float insideValue = 255, 
            outsideValue = 0;


/* Statistics */
typedef itk::StatisticsImageFilter<ImageType> 
    StatsImageFilterType;

void countPixels (ImageType::Pointer im);


/* Binary Morphology */
typedef itk::BinaryBallStructuringElement<float, Dim>
    StructuringElementType; 

typedef itk::BinaryErodeImageFilter< 
    ImageType, 
    ImageType, 
    StructuringElementType > ErodeFilterType;

typedef itk::BinaryDilateImageFilter< 
    ImageType, 
    ImageType, 
    StructuringElementType > DilateFilterType;

typedef itk::CastImageFilter<ImageType, OutputImageType> 
    CastImageFilterType;


/* Connected components */
typedef itk::ConnectedComponentImageFilter<OutputImageType, OutputImageType>
    ConnectedComponentImageFilterType; 

typedef itk::RelabelComponentImageFilter<OutputImageType, OutputImageType>
    RelabelComponentImageFilterType;

typedef itk::BinaryImageToLabelMapFilter<OutputImageType> 
    BinaryImageToLabelMapFilterType;

typedef itk::LabelMapToLabelImageFilter<BinaryImageToLabelMapFilterType::OutputImageType, OutputImageType>
    LabelMapToLabelImageFilterType;

typedef itk::LabelGeometryImageFilter< OutputImageType > LabelGeometryImageFilterType; 

typedef itk::ImageRegionIterator<ImageType> IteratorType;

/* Miscellaneous */
typedef itk::AddImageFilter<ImageType, ImageType, ImageType> AddImageFilterType; 




/**
 * Higher order routines. Here we abstract the functionality of ITK within
 * the context of top-level programs, i.e. detect, filter, binmorph, etc. 
 * This takes care of assembling image processing pipelines. 
 */
class Blob; 
struct param_t; 
 
ImageType::Pointer delta( const char *in1, 
                          const char *in2, 
                          bool thresh, const param_t &options );

ImageType::Pointer threshold( const char *in, const param_t &options );

ImageType::Pointer read( const char *in, const param_t &options ); 

ImageType::Pointer delta( ImageType::Pointer &im1, 
                          ImageType::Pointer &im2, 
                          bool thresh, const param_t &options );

ImageType::Pointer delta( ImageType::Pointer &im1, 
                          ImageType::Pointer &im2, 
                          const Blob &blob ); 
                          
ImageType::Pointer threshold( ImageType::Pointer &im, const param_t &options ); 

ImageType::Pointer morphology( ImageType::Pointer &im, const param_t &options ); 

void write( ImageType::Pointer &im, const char *out ); 

void copy( ImageType::Pointer &im1, const ImageType::Pointer &im2 ); 

void connectedComponents( 
  ImageType::Pointer &im, 
  std::vector<RelabelComponentImageFilterType::ObjectSizeType> &sizes );

int getBlobs( ImageType::Pointer &im, std::vector<Blob> &blobs ); 

void drawBoundingBox( const char *in, const char *out, const Blob &blob );

#endif
