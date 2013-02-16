/* Christopher Patton
 * John Muir Institute for the Environment
 * University of California, Davis
 * June 2012
 * 
 * Header for ITK includes and type declarations. This file 
 * also declares functions that implement ITK pipelines for
 * the salamander project. 
 *
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
 * Generate normally distributed samples over a range of indices.
 */ 
void sample(std::vector<int> &samples, int ct, int i, int j, int mean, int sd); 

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
