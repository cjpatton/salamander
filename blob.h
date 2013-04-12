/* John Muir Institute for the Environment
 * University of California, Davis
 * 
 * blob.h
 * Data structure for representing "blobs". This file is part of the 
 * Salamander project. 
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


#ifndef BLOB_H
#define BLOB_H

#include "salamander.h"

class Blob {
friend std::ostream& operator<< (std::ostream&, const Blob&); 

  /* Bounding box is used for tracking targets. */
  
  int bbox [4];                 /* bounding box */ 
  int frameWidth, frameHeight;  /* dimensions of video stream */  

  /* Blob centroid, elongation, and volume. These will be used for more
   * sophisticated detection techniques, e.g., when many blobs correspond
   * to one large target. */

  int centroidX, centroidY;    
  double elongation; 
  int volume; 

  bool containsPoint(int x, int y) const; 

public: 

  Blob(); 

  Blob( int top, int right, int bottom, int left ); 

  /* Constructor for blobs created via connected component analysis. 
   * See salamander.cxx::getBlobs(). */ 

  //Blob( LabelGeometryImageFilterType::LabelPointType c, double e, int v, 
  //      int w, int h ); 

  /* Accessors and modifiers for bounding box. These routines implement
   * features for tracking. */ 

  Blob  operator*(int scale) const; 
  Blob& operator=(const Blob &blob); 
  bool operator==(const Blob &blob) const; 
  bool operator!=(const Blob &blob) const; 
  int& operator[](int i); 
  int operator[](int i) const;
  bool Contains(const Blob &blob) const; 
  int Intersects( const Blob &blob ) const; 
  void shiftOverMerged( const Blob &merged ); 
  int GetBoundingBoxArea() const; 

  /* Return an ITK type region for cropping an area in an image bounded
   * by a blob. */
   
  //void GetRegion(ImageType::RegionType &region) const; 
  
  /* Accessors for tracking parameters */ 
   
  int DistanceTo( const Blob &blob ) const;
  int GetCentroidX() const; 
  int GetCentroidY() const; 
  double GetElongation () const; 
  int GetVolume() const; 
  
}; 

std::ostream& operator<< (std::ostream &out, const Blob &blob); 

#endif // BLOB_H
