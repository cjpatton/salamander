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
#define UNASSIGNED -1
#define MAXCOMPS 1024

class ConnectedComponents; 

class Blob {
friend std::ostream& operator<< (std::ostream&, const Blob&); 
friend class ConnectedComponents; 

  /* Bounding box is used for tracking targets. */
  
  int bbox [4];                 /* bounding box */ 
  int frameWidth, frameHeight;  /* dimensions of video stream */  

  /* Blob centroid, elongation, and volume. These will be used for more
   * sophisticated detection techniques, e.g., when many blobs correspond
   * to one large target. */

  int centroid_x, centroid_y;    
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
   
  cv::Rect GetRegion() const; 
  
  /* Accessors for tracking parameters */ 
   
  int DistanceTo( const Blob &blob ) const;
  int GetCentroidX() const; 
  int GetCentroidY() const; 
  int GetVolume() const; 
  
}; 

std::ostream& operator<< (std::ostream &out, const Blob &blob); 


class ConnectedComponents
{

public: 

  ConnectedComponents( const cv::Mat& ); 
  ~ConnectedComponents(); 
  
  /* Write labeled image to file */ 
  void write( const char *fn ); 

  /* Label image and emit a reference to it */ 
  cv::Mat &labeled(); 

  /* Debugging display */ 
  void disp() const; 

  struct component_t; 

  /* Disjoint-set data structure and cc matrix cell */ 
  struct label_t {
    
    label_t() {
      label = UNASSIGNED; 
      parent = NULL; 
      component = NULL; 
    }

    uchar pixel; 
    int   label;
    label_t *parent;
    component_t *component; 
  };

  struct component_t {

    component_t() {
      label = NULL; 
      blob.volume = 0; 
      blob.centroid_x = blob.centroid_y = 0; 
    }

    label_t *label; 
    Blob blob; 
  }; 
  
  /* Component accessors */ 
  Blob &operator[] (int); 
  const Blob &operator[] (int) const; 
  int size() const; 
    
private:

  /* Disjoint-set methods */
  void      _union (label_t &a, label_t &b);
  label_t & _find (label_t &a) const; 
    
  /* Connected component analysis */ 
  int getneighbors(label_t* neighbors [], int i, int j);
  void ccomp();
  
  label_t     *labels; 
  component_t *components; 
  int rows, cols, components_ct; 
  
  cv::Mat img; 
   
}; // class ConnectedComponents
#endif // BLOB_H
