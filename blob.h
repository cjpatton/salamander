#ifndef BLOB_H
#define BLOB_H

#include "salamander.h"

/* Datatype for blobs */
class Blob {
friend std::ostream& operator<< (std::ostream&, const Blob&); 

  int bbox [4];                 /* bounding box */ 
  int frameWidth, frameHeight;  /* dimensions of video stream */  
  ImageType::RegionType region;

  /* relevant? */ 
  int centroidX, centroidY; 
  double elongation; 
  int volume; 

  bool containsPoint(int x, int y) const; 

public: 

  Blob(); 

  Blob( int top, int right, int bottom, int left ); 

  Blob( LabelGeometryImageFilterType::BoundingBoxType b,
        LabelGeometryImageFilterType::LabelPointType c, double e, int v,
        int w, int h );

  Blob  operator*(int scale) const; 
  Blob& operator=(const Blob &blob); 
  bool operator==(const Blob &blob) const; 
  bool operator!=(const Blob &blob) const; 
  int& operator[](int i); 
  int operator[](int i) const;
  bool Contains(const Blob &blob) const; 

  void GetRegion(ImageType::RegionType &region) const; 
  int GetBoundingBoxArea() const; 
  int DistanceTo( const Blob &blob ) const;
  int Intersects( const Blob &blob ) const; 
  void shiftOverMerged( const Blob &merged ); /* TODO better name */ 
  
  /* relevant? */
  int GetCentroidX() const; 
  int GetCentroidY() const; 
  double GetElongation () const; 
  int GetVolume() const; 
  
}; 

std::ostream& operator<< (std::ostream &out, const Blob &blob); 

#endif // BLOB_H
