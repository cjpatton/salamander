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
#include <algorithm> // sort()
#include <cstring>
#include <cstdlib>

void countPixels(ImageType::Pointer im) 
/* count the frequency of each pixel color (gray scale images) */ 
{
    static int pixels [256];
    memset(pixels, 0, sizeof(int) * 256); 
    IteratorType it( im, im->GetRequestedRegion() );
    for (it = it.Begin(); !it.IsAtEnd(); ++it) {
        if ((int)it.Get() < 256)
            pixels[(int)it.Get()] ++; 
        else
            std::cout << "pixel out of range\n";
    }
    for (int i = 0; i < 256; i++) {
        if (pixels[i] > 0)
            std::cout << i << ": " << pixels[i] << std::endl;
    }
}


ImageType::Pointer delta( const char *in1, 
                          const char *in2, 
                          bool thresh, const param_t &options ) 
/* Subtract a video frame from the previous in the stream and apply
 * binary morphology filter. */
{
    /* readers */
    Reader::Pointer reader1 = Reader::New();
    reader1->SetFileName( in1 );
   
    Reader::Pointer reader2 = Reader::New();
    reader2->SetFileName( in2 );

    /* converters */
    ConverterType::Pointer conv1 = ConverterType::New(); 
    conv1->SetInput( reader1->GetOutput() );

    ConverterType::Pointer conv2 = ConverterType::New(); 
    conv2->SetInput( reader2->GetOutput() );

    /* shrink image filters */
    ShrinkImageFilterType::Pointer shrinker1 = ShrinkImageFilterType::New(); 
    shrinker1->SetInput( conv1->GetOutput() ); 
    shrinker1->SetShrinkFactor(0, options.shrink_factor); 
    shrinker1->SetShrinkFactor(1, options.shrink_factor);
    
    ShrinkImageFilterType::Pointer shrinker2 = ShrinkImageFilterType::New(); 
    shrinker2->SetInput( conv2->GetOutput() ); 
    shrinker2->SetShrinkFactor(0, options.shrink_factor); 
    shrinker2->SetShrinkFactor(1, options.shrink_factor);

    /* subtract image from next in series */
    SubtractImageFilterType::Pointer subtractor = SubtractImageFilterType::New(); 
    subtractor->SetInput1( shrinker1->GetOutput() );
    subtractor->SetInput2( shrinker2->GetOutput() ); 

    /* absolute value */
    AbsImageFilterType::Pointer abs = AbsImageFilterType::New();
    abs->SetInput( subtractor->GetOutput() );
    
    /* binary threshold filter */
    BinaryThresholdImageFilterType::Pointer 
        threshold = BinaryThresholdImageFilterType::New(); 
    threshold->SetInput( abs->GetOutput() );
    threshold->SetOutsideValue( outsideValue );
    threshold->SetInsideValue(  insideValue  );
    threshold->SetLowerThreshold( options.low );
    threshold->SetUpperThreshold( options.high ); 

    if (thresh) {
        threshold->Update(); 
        return threshold->GetOutput();
    } else {
        abs->Update(); 
        return abs->GetOutput(); 
    }
      

    /* statistics 
    StatsImageFilterType::Pointer stats = StatsImageFilterType::New();
    stats->SetInput( abs->GetOutput() );
    stats->Update();

    //std::cout << " things\n";

    //countPixels( abs->GetOutput() ); 

    sd::cout //<< "\n" 
        //<< "min:  " << stats->GetMinimum() << std::endl
        //<< "max:  " << stats->GetMaximum() << std::endl
        << "mean: " << stats->GetMean() << std::endl;*/
   
} // original delta() 

ImageType::Pointer threshold( const char *in, const param_t &options ) 
/* Binary threshold. */ 
{
    /* readers */
    Reader::Pointer reader = Reader::New();
    reader->SetFileName( in );
   
    /* converters */
    ConverterType::Pointer conv = ConverterType::New(); 
    conv->SetInput( reader->GetOutput() );

    /* binary threshold filter */
    BinaryThresholdImageFilterType::Pointer 
        threshold = BinaryThresholdImageFilterType::New(); 
    threshold->SetInput( conv->GetOutput() );
    threshold->SetOutsideValue( outsideValue );
    threshold->SetInsideValue(  insideValue  );
    threshold->SetLowerThreshold( options.low );
    threshold->SetUpperThreshold( options.high ); 

    /* absolute value */
    AbsImageFilterType::Pointer abs = AbsImageFilterType::New();
    abs->SetInput( threshold->GetOutput() );
    
    abs->Update();
    return abs->GetOutput();
} // threshold()

ImageType::Pointer read( const char *in, const param_t &options ) 
/* Read and convert an image for the processing pipeline. */
{
    Reader::Pointer reader = Reader::New();
    reader->SetFileName( in );
   
    ConverterType::Pointer conv = ConverterType::New(); 
    conv->SetInput( reader->GetOutput() );

    /* shrink image filter */
    ShrinkImageFilterType::Pointer shrinker = ShrinkImageFilterType::New(); 
    shrinker->SetInput( conv->GetOutput() ); 
    shrinker->SetShrinkFactor(0, options.shrink_factor);
    shrinker->SetShrinkFactor(1, options.shrink_factor);

    shrinker->Update(); 
    return shrinker->GetOutput(); 
} // read() 

ImageType::Pointer delta( ImageType::Pointer &im1, 
                          ImageType::Pointer &im2, 
                          bool thresh, const param_t &options ) 
/* Subtrac a video frame from prevoius in stream and apply binary threshold. */
{

    /* subtract image from next in series */
    SubtractImageFilterType::Pointer subtractor = SubtractImageFilterType::New(); 
    subtractor->SetInput1( im1 );
    subtractor->SetInput2( im2 ); 

    /* absolute value */
    AbsImageFilterType::Pointer abs = AbsImageFilterType::New();
    abs->SetInput( subtractor->GetOutput() );
    
    /* binary threshold filter */
    BinaryThresholdImageFilterType::Pointer 
        threshold = BinaryThresholdImageFilterType::New(); 
    threshold->SetInput( abs->GetOutput() );
    threshold->SetOutsideValue( outsideValue );
    threshold->SetInsideValue(  insideValue  );
    threshold->SetLowerThreshold( options.low );
    threshold->SetUpperThreshold( options.high ); 

    if (thresh) {
        threshold->Update(); 
        return threshold->GetOutput();
    } else {
        abs->Update(); 
        return abs->GetOutput(); 
    }
} // delata() 

ImageType::Pointer delta( ImageType::Pointer &im1, 
                          ImageType::Pointer &im2, 
                          const Blob &blob ) 
/* Just calculate delta. */ 
{

    /* Region of Interest filter */
    ROIFilterType::Pointer filter1 = ROIFilterType::New(), 
                           filter2 = ROIFilterType::New();

    ImageType::RegionType region;
    blob.GetRegion( region ); 

    filter1->SetRegionOfInterest( region ); 
    filter2->SetRegionOfInterest( region ); 

    filter1->SetInput( im1 );
    filter2->SetInput( im2 );

    /*filter1->Update();
    ImageType::Pointer poop1 = filter1->GetOutput(); 
    write( poop1, "apple1.jpg");
    filter2->Update();
    ImageType::Pointer poop2 = filter2->GetOutput(); 
    write( poop2, "apple2.jpg");*/

    /* subtract image from next in series */
    SubtractImageFilterType::Pointer subtractor = SubtractImageFilterType::New(); 
    subtractor->SetInput1( filter1->GetOutput() );
    subtractor->SetInput2( filter2->GetOutput() ); 
    subtractor->GetOutput()->SetRequestedRegion( region ); 

    /* absolute value */
    AbsImageFilterType::Pointer abs = AbsImageFilterType::New();
    abs->SetInput( subtractor->GetOutput() );

    abs->Update(); 
    return abs->GetOutput();
} // delta() 


ImageType::Pointer threshold( ImageType::Pointer &im, const param_t &options ) 
/* Apply binary threshold filter to delta. */  
{
   
    /* binary threshold filter */
    BinaryThresholdImageFilterType::Pointer 
        threshold = BinaryThresholdImageFilterType::New(); 
    threshold->SetInput( im );
    threshold->SetOutsideValue( outsideValue );
    threshold->SetInsideValue(  insideValue  );
    threshold->SetLowerThreshold( options.low );
    threshold->SetUpperThreshold( options.high ); 

    threshold->Update(); 
    return threshold->GetOutput(); 
} // threshold() 

ImageType::Pointer morphology( ImageType::Pointer &im, const param_t &options ) 
/* Apply binary morphology filter to delta. Erode away weak blobs and dilate 
 * the remaining. */
{
    ErodeFilterType::Pointer    erode = ErodeFilterType::New();
    DilateFilterType::Pointer   dilate = DilateFilterType::New();
  
    /* erode */
    StructuringElementType erode_element;
    erode_element.SetRadius( options.erode );
    erode_element.CreateStructuringElement();
    erode->SetKernel( erode_element );

    /* dilate */
    StructuringElementType dilate_element;
    dilate_element.SetRadius( options.dilate );
    dilate_element.CreateStructuringElement();
    dilate->SetKernel( dilate_element );

    erode->SetErodeValue( insideValue );
    erode->SetInput( im );  

    dilate->SetDilateValue( insideValue );
    dilate->SetInput( erode->GetOutput() );

    dilate->Update(); 
    return dilate->GetOutput();
} // threshold() 

void copy( ImageType::Pointer &im1, const ImageType::Pointer &im2 ) 
/* Duplicate an image. */ 
{
  DuplicatorType::Pointer duplicator = DuplicatorType::New();
  duplicator->SetInputImage(im2); 
  duplicator->Update();
  im1 = duplicator->GetOutput();  
} // copy() 

void write( ImageType::Pointer &im, const char *out ) 
/* Cast an image to JPEG compatible output and write to disk. */
{
    /* make sure to output in correct JPEG format */
    CastImageFilterType::Pointer caster = CastImageFilterType::New();
    caster->SetInput( im );

    /* writer */
    Writer::Pointer writer = Writer::New();
    writer->SetInput( caster->GetOutput() );
    writer->SetFileName( out );
    writer->Update();
} // write() 


void connectedComponents( 
 ImageType::Pointer &im, 
 std::vector<RelabelComponentImageFilterType::ObjectSizeType> &sizes ) 
/* Simple (fast) connected component anaylysis. Return a vector of blob sizes. */ 
{
    sizes.clear(); 
  
    /* make sure to output in correct JPEG format */
    CastImageFilterType::Pointer caster = CastImageFilterType::New();
    caster->SetInput( im ); 

    /* connected components */
    ConnectedComponentImageFilterType::Pointer
        components = ConnectedComponentImageFilterType::New();
    components->SetInput( caster->GetOutput() );

    RelabelComponentImageFilterType::Pointer
        relabel = RelabelComponentImageFilterType::New(); 
    relabel->SetInput( components->GetOutput() ); 
    relabel->Update();

    sizes = relabel->GetSizeOfObjectsInPixels(); 
} // connectedComponents() 


int getBlobs( ImageType::Pointer &im, std::vector<Blob> &blobs ) 
/* Connected component analysis, but give more information about the blobs. 
 * Return a vector of Blob objects with bounding box, centroid, volume (size), 
 * and elongation. */ 
{
  int i; 
  
  CastImageFilterType::Pointer caster = CastImageFilterType::New();
  caster->SetInput( im ); 
  
  BinaryImageToLabelMapFilterType::Pointer labelMap = 
                                   BinaryImageToLabelMapFilterType::New(); 
  labelMap->SetInput( caster->GetOutput() ); 
  labelMap->Update(); 

  LabelMapToLabelImageFilterType::Pointer labelImage = 
                                    LabelMapToLabelImageFilterType::New();
  labelImage->SetInput( labelMap->GetOutput() );
  labelImage->Update(); 

  LabelGeometryImageFilterType::Pointer geo = 
                                      LabelGeometryImageFilterType::New(); 
  geo->SetInput( labelImage->GetOutput() ); 

  // These generate optional outputs.
  //geo->CalculatePixelIndicesOn();
  //geo->CalculateOrientedBoundingBoxOn();
  //geo->CalculateOrientedLabelRegionsOn();
 
  geo->Update();
 
  LabelGeometryImageFilterType::LabelsType allLabels = geo->GetLabels();
  LabelGeometryImageFilterType::LabelsType::iterator allLabelsIt;
  //std::cout << "Number of labels: " << geo->GetNumberOfLabels() << std::endl;
 
  blobs.clear(); 
  for( allLabelsIt = allLabels.begin()+1; allLabelsIt != allLabels.end(); allLabelsIt++ )
  {
    LabelGeometryImageFilterType::LabelPixelType labelValue = *allLabelsIt;
    //std::cout << "\tVolume: " << geo->GetVolume(labelValue) << std::endl;
    //std::cout << "\tIntegrated Intensity: " << geo->GetIntegratedIntensity(labelValue) << std::endl;
    //std::cout << "\tCentroid: " << geo->GetCentroid(labelValue) << std::endl;
    //std::cout << "\tWeighted Centroid: " << geo->GetWeightedCentroid(labelValue) << std::endl;
    //std::cout << "\tAxes Length: " << geo->GetAxesLength(labelValue) << std::endl;
    //std::cout << "\tMajorAxisLength: " << geo->GetMajorAxisLength(labelValue) << std::endl;
    //std::cout << "\tMinorAxisLength: " << geo->GetMinorAxisLength(labelValue) << std::endl;
    //std::cout << "\tEccentricity: " << geo->GetEccentricity(labelValue) << std::endl;
    //std::cout << "\tElongation: " << geo->GetElongation(labelValue) << std::endl;
    //std::cout << "\tOrientation: " << geo->GetOrientation(labelValue) << std::endl;
    //std::cout << "\tBounding box: " << geo->GetBoundingBox(labelValue) << std::endl;
    blobs.push_back( Blob(geo->GetBoundingBox(labelValue), 
                     geo->GetCentroid(labelValue),
                     geo->GetElongation(labelValue),
                     geo->GetVolume(labelValue),
                     im->GetLargestPossibleRegion().GetSize()[0],
                     im->GetLargestPossibleRegion().GetSize()[1]) ); 
  }

  return blobs.size(); 
} // getBlobs() 

void drawBoundingBox( const char *in, const char *out, const Blob &blob ) 
/* Draw a bounding box on a JPEG image, as specified by a Blob object. Output
 * to a new file. */ 
{

  Reader::Pointer reader = Reader::New();
  reader->SetFileName( in );
  reader->Update(); 
  
  InputPixelType pixel;
  pixel.SetRed(255); 
  pixel.SetGreen(50); 
  pixel.SetBlue(0); 

  InputImageType::IndexType a, b; 
  typedef itk::LineIterator<InputImageType> IteratorType;
  IteratorType it(reader->GetOutput(), a, b); 

  /* top */ 
  a[0] = blob[0]; 
  b[0] = blob[1]; 
  a[1] = b[1] = blob[2]; 
  it = IteratorType(reader->GetOutput(), a, b); 
  it.GoToBegin(); 
  while(!it.IsAtEnd()) {
    it.Set(pixel); 
    ++it; 
  }
  
  /* left */ 
  a[0] = b[0] = blob[0]; 
  a[1] = blob[2]; 
  b[1] = blob[3]; 
  it = IteratorType(reader->GetOutput(), a, b); 
  it.GoToBegin(); 
  while(!it.IsAtEnd()) {
    it.Set(pixel); 
    ++it; 
  }

  /* right */ 
  a[0] = b[0] = blob[1]; 
  a[1] = blob[2]; 
  b[1] = blob[3]; 
  it = IteratorType(reader->GetOutput(), a, b); 
  it.GoToBegin(); 
  while(!it.IsAtEnd()) {
    it.Set(pixel); 
    ++it; 
  }
  
  /* bottom */ 
  a[0] = blob[0]; 
  b[0] = blob[1]; 
  a[1] = b[1] = blob[3]; 
  it = IteratorType(reader->GetOutput(), a, b); 
  it.GoToBegin(); 
  while(!it.IsAtEnd()) {
    it.Set(pixel); 
    ++it; 
  }
  
  typedef itk::ImageFileWriter<InputImageType> RGBWriter; 
  RGBWriter::Pointer writer = RGBWriter::New();
  writer->SetInput( reader->GetOutput() );
  writer->SetFileName( out );
  writer->Update();
  
} // drawBoundingBox() 
