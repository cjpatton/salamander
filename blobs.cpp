/* John Muir Institute for the Environment
 * University of California, Davis
 * 
 * blobs.cpp
 * Data structures for representing "blobs" and implementation of 
 * connected component analysis. This file is part of the 
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

#include "salamander.h"
#include "blobs.h"
#include <iostream>

#define min(x,y) (x < y ? x : y)
#define max(x,y) (x < y ? y : x)

Blob::Blob() {
  frame_width = frame_height = 0; 
  bbox[0] = 0;
  bbox[1] = 0;
  bbox[2] = 0;
  bbox[3] = 0;
  centroid_x = centroid_y = 0; 
  volume = 0; 
} // dumb constr

//Blob::Blob( LabelGeometryImageFilterType::BoundingBoxType b,
//            LabelGeometryImageFilterType::LabelPointType c, 
//            double e, int v, 
//            int w, int h )
//{
//  /* bounding box corners: 
//   * (0,2) (1,2)
//   * (0,3) (1,3) */
//
//  frame_width = w; 
//  frame_height = h; 
//
//  bbox[0] = b[0];
//  bbox[1] = b[1];
//  bbox[2] = b[2];
//  bbox[3] = b[3];
//
//  centroidX = c[0]; 
//  centroidY = c[1];
//
//  elongation = e; 
//  volume = v;    
//} // salamander.cpp::getBlobs() constr

Blob::Blob( int top, int right, int bottom, int left ) 
{
  frame_width = frame_height = 0; 
  bbox[0] = left;
  bbox[1] = right;
  bbox[2] = top;
  bbox[3] = bottom;
  
  centroid_x = centroid_y = 0; 
  volume = 0; 
} // bounding box constr


Blob Blob::operator*(int scale) const 
/* Images can be scaled in order to improve efficiency. This operator 
 * is useful for rescaling blobs the original image. Say a scaling factor
 * of 3 is used. Then salamander.cpp::drawBoundingBox() should be used 
 * like drawBoundingBox(original_image, target_blob * 3). */ 
{
  Blob newBlob = *this;
  newBlob.frame_height *= scale; 
  newBlob.frame_width *= scale; 
  newBlob.bbox[0] = max(0, bbox[0]*scale); 
  newBlob.bbox[1] = min(newBlob.frame_width-1, bbox[1]*scale); 
  newBlob.bbox[2] = max(0, bbox[2]*scale); 
  newBlob.bbox[3] = min(newBlob.frame_height-1, bbox[3]*scale);  
  return newBlob; 
} // operator* 
  
Blob& Blob::operator=(const Blob &blob) 
{
  frame_width = blob.frame_width; 
  frame_height = blob.frame_height; 

  bbox[0] = blob.bbox[0];
  bbox[1] = blob.bbox[1];
  bbox[2] = blob.bbox[2];
  bbox[3] = blob.bbox[3];

  centroid_x = blob.centroid_x; 
  centroid_y = blob.centroid_y;

  volume = blob.volume;    
  return *this; 
} // operator=

bool Blob::operator==(const Blob &blob) const 
/* This is not mean to express precise equality, since this is rarely 
 * useful in tracking. Instead, we say two blobs are equivalent if they are 
 * roughly the same size and their intersection is large. */ 
{
  double aIntersect = (double)Intersects(blob),
         aRatio = aIntersect/GetBoundingBoxArea(),
         aBlobRatio = aIntersect/blob.GetBoundingBoxArea();

  //std::cout << aIntersect << " " << aRatio << " " << aBlobRatio << std::endl;

  if (abs(aRatio - aBlobRatio) < 0.25 && aRatio > 0.75 && aBlobRatio > 0.75)
    return true;

  else return false; 
} // operator==

bool Blob::operator!=(const Blob &blob) const 
{
  return !(*this == blob);
} // operator!=


int& Blob::operator[](int i) 
{
  return bbox[i]; 
} // operator[]

int Blob::operator[](int i) const 
{
  return bbox[i]; 
} // operator[] const 



bool Blob::Contains(const Blob &blob) const 
/* Blob A contains blob B if their intersection is equal to the
 * size of blob B. */ 
{
  if (Intersects(blob) == blob.GetBoundingBoxArea())
    return true;
  else return false; 
} // Contains()

int Blob::Intersects( const Blob &blob ) const 
/* Return area of blob intersection. If bounding box is smaller than the 
 * sum of the heights and widths of the blobs, then they intersect. Assume 
 * the origin is in the top left corner. Orientation of bounding box is:  
   * top left (0,2) (1,2)
   *          (0,3) (1,3) bottom right */
{
  int width = max(bbox[1], blob.bbox[1]) - min(bbox[0], blob.bbox[0]); 
  int height = max(bbox[3], blob.bbox[3]) - min(bbox[2], blob.bbox[2]); 

  if (width < (bbox[1] - bbox[0] + blob.bbox[1] - blob.bbox[0]) && 
      height < (bbox[3] - bbox[2] + blob.bbox[3] - blob.bbox[2]))
    return (bbox[1] - bbox[0] + blob.bbox[1] - blob.bbox[0] - width) *  
           (bbox[3] - bbox[2] + blob.bbox[3] - blob.bbox[2] - height); 

  else return 0;  
} // Intersects() 

void Blob::shiftOverMerged( const Blob &merged ) 
/* Shift blob over merged when tracking a target. When a target only moves 
 * a little between two frames, a single blob may correspond to the new 
 * position and the old position in the delta window. This funciton infers 
 * which corner of the merged blob corresponds to the old one and shifts 
 * the bounding box diagonally across the merged blob. */ 
{

  /* It's possible the target's orientation has changed. Make the new 
   * bounding box a square whose side equals the longest side of the old
   * blob. That way we guarentee the target appears within the box. */ 
  int new_width = max(bbox[1]-bbox[0], bbox[3]-bbox[2]); 

  /* Find the box corners that are nearest to each other */ 
  int i, j, dist, 
        mini=-1, minj=-1, mindist = 10000000;
  for (i = 0; i < 2; i++) {
    for (j = 2; j < 4; j++) {
      dist = sqrt(pow(bbox[i] - merged.bbox[i], 2) + 
                  pow(bbox[j] - merged.bbox[j], 2)); 
      if (dist < mindist) {
        mindist = dist;
        mini = i;
        minj = j;
      }
    }
  } 
  //std::cout<<"    nearest corner: " << mini << ' ' << minj << std::endl;
  
  /* Sometimes the merged blob is smaller. In this case, we can merge it 
   * over the old blob and still achieve a high degree of accuracy. */ 
  Blob smaller, bigger; 
  if (GetBoundingBoxArea() < merged.GetBoundingBoxArea()) {
    smaller = *this; 
    bigger = merged; 
  } else { 
    smaller = merged; 
    bigger =   *this; 
  }
  
  /* Shift blob diagonally over merged */ 
  
  if (mini == 0 && minj == 2) { /* top left */ 
    smaller.bbox[1] = bigger.bbox[1]; 
    smaller.bbox[0] = bigger.bbox[1] - new_width; 
    smaller.bbox[3] = bigger.bbox[3];
    smaller.bbox[2] = bigger.bbox[3] - new_width; 
    *this = smaller; 
  }

  else if (mini == 1 && minj == 2) { /* top right */ 
    smaller.bbox[0] = bigger.bbox[0]; 
    smaller.bbox[1] = bigger.bbox[0] + new_width; 
    smaller.bbox[3] = bigger.bbox[3];
    smaller.bbox[2] = bigger.bbox[3] - new_width;
    *this = smaller; 
  }
  
  else if (mini == 0 && minj == 3) { /* bottom left */ 
    smaller.bbox[1] = bigger.bbox[1]; 
    smaller.bbox[0] = bigger.bbox[1] - new_width; 
    smaller.bbox[2] = bigger.bbox[2];
    smaller.bbox[3] = bigger.bbox[2] + new_width; 
    *this = smaller; 
  }

  else if (mini == 1 && minj == 3) { /* bottom right */ 
    smaller.bbox[0] = bigger.bbox[0]; 
    smaller.bbox[1] = bigger.bbox[0] + new_width; 
    smaller.bbox[2] = bigger.bbox[2];
    smaller.bbox[3] = bigger.bbox[2] + new_width; 
    *this = smaller; 
  }

  else { 
    std::cout << "\t\t\tI dong it\n";
    *this = merged; /* I wonder if this will happen. */
  }

  /* Keep new blob within frame. */
  bbox[0] = max(0, bbox[0]);
  bbox[1] = min(frame_width, bbox[1]); 
  bbox[2] = max(0, bbox[2]); 
  bbox[3] = min(frame_height, bbox[3]); 
  
} // shiftOverMerged()

int Blob::GetBoundingBoxArea() const 
{
  return ((bbox[1]-bbox[0]) * (bbox[3]-bbox[2])); 
} // GetBoundingBoxArea()



cv::Rect Blob::GetRegion() const
///* Create ITK region from bounding box */ 
{
   /* top left (0,2) (1,2)
    *          (0,3) (1,3) bottom right */

  return cv::Rect(bbox[0], bbox[2], bbox[1] - bbox[0], bbox[3] - bbox[2]); 

} // GetRegion()



int Blob::DistanceTo( const Blob &blob ) const 
/* Euclidean distance between blob centroids */ 
{
  return sqrt(pow(blob.centroid_y - centroid_y, 2) + 
                              pow(blob.centroid_x - centroid_x, 2));
} // DistanceTo() 

int Blob::GetCentroidX() const 
{
  return centroid_x; 
} // GetCentroidX()

int Blob::GetCentroidY() const 
{
  return centroid_y; 
} // GetCentroidY() 

int Blob::GetVolume() const 
{
  return volume; 
} // GetVolume()

std::ostream& operator<< (std::ostream &out, const Blob &blob) 
{
  out //<< " centroid (" << blob.centroidX << ", " << blob.centroidY << ')'
      //<< " volume " << blob.volume << " elongation " << blob.elongation
      << " [" << blob.bbox[0] << ", " << blob.bbox[1] 
      << ", " << blob.bbox[2] << ", " << blob.bbox[3] << "]";
  return out; 
} // operator<<

bool Blob::containsPoint( int x, int y ) const 
/* Bounding box contains point (x,y) */
{
  return (x >= bbox[0] && x <= bbox[1] && y >= bbox[2] && y <= bbox[3]);  
} // containsPoint() 




ConnectedComponents::ConnectedComponents( const cv::Mat &anImage )
{
  CV_Assert(anImage.depth() == CV_8U);  // accept only uchar images
  CV_Assert(anImage.channels() == 1);   // just one channel

  img = anImage.clone();
  rows = img.rows; 
  cols = img.cols;
  components_ct = 0; 
  
  labels = new label_t [rows * cols]; 
  components = new component_t [MAXCOMPS]; 

  int nrows = img.rows;
  int ncols = img.cols;

  if (img.isContinuous())
  {
    ncols *= nrows;
    nrows = 1;
  }

  // populate label matrix
  int i, j, ct, label = 0; 
  uchar* p;
  for (i = 0; i < nrows; ++i)
  {
    label_t &q = labels[i * cols + j]; 
    p = img.ptr<uchar>(i);
    for (j = 0; j < ncols; ++j)
    {
      labels[i * cols + j].pixel = p[j];
    }
  }
  
  // perform connected component analysis
  ccomp(); 
  
} // constr

ConnectedComponents::~ConnectedComponents() 
{
  delete [] labels;
  delete [] components;
} // destr
  
/* Component accessors */ 
Blob &ConnectedComponents::operator[] (int i) 
{
  assert(i >= 0 && i < components_ct); 
  return components[i].blob; 
} // operator[]

const Blob &ConnectedComponents::operator[] (int i) const
{
  assert(i >= 0 && i < components_ct); 
  return components[i].blob; 
} // operator[] const 

int ConnectedComponents::size() const
{
  return components_ct; 
} // site()

void ConnectedComponents::disp() const 
{
  int i,j; 
  // print
  for (i = 0; i < rows; i++) 
  {
    for (j = 0; j < cols; j++) 
    { 
      //printf("(%-1d,%-3d):", labels[i * cols +j].x, labels[i * cols + j].y); 
      if (labels[i * cols +j].label != UNASSIGNED)
        printf("%2d  ", labels[i * cols + j].label);
      else
        printf(" -  "); 
    }
    printf("\n"); 
  }

  for (i = 0; i < components_ct; i++) 
    printf("%d (%d, %d) vol=%d blob.bbox=[%d, %d, %d, %d]\n", components[i].label->label, 
        components[i].blob.centroid_x, components[i].blob.centroid_y, components[i].blob.volume,
        components[i].blob.bbox[0], components[i].blob.bbox[1], 
        components[i].blob.bbox[2], components[i].blob.bbox[3] ); 
} // disp() 


cv::Mat& ConnectedComponents::labeled() 
{
  int nrows = rows;
  int ncols = cols;

  if (img.isContinuous())
  {
    ncols *= nrows;
    nrows = 1;
  }
  int i, j, ct, label = 0; 

  uchar* p;
  for (i = 0; i < nrows; ++i)
  {
    label_t &q = labels[i * cols + j]; 
    p = img.ptr<uchar>(i);
    for (j = 0; j < ncols; ++j)
    {
      p[j] = (labels[i * cols + j].pixel * 10) % 255; /* FIXME */ 
    }
  }
  return img; 
} // labeled() 

void ConnectedComponents::write( const char *fn ) 
{
  cv::imwrite( fn, labeled() ); 
} // write()


void ConnectedComponents::ccomp() 
{
  int i, j, ct, label = 0; 
  label_t *neighbors [8]; 
  
  // first pass label 
  for (i = 0; i < rows; i++) 
  {
    for (j = 0; j < cols; j++) 
    {
      label_t &q = labels[i * cols + j]; 
      if (q.pixel > 0)
      {
        ct = getneighbors(neighbors, i, j);
        switch(ct) 
        {
          case 0: 
            q.label = label++;                // root
            break;

          default: 
            label_t *min = neighbors[0]; 
            for (int k = 1; k < ct; k++) 
            {
              if (neighbors[k]->label < min->label)
                min = neighbors[k]; 
            }
            q.label  = min->label;             // sibling of min label
            q.parent = min->parent; 

            for (int k = 0; k < ct; k++)       // neighbors are in the same group
              _union(q, *neighbors[k]); 

        }
      }
    }
  }

  // second pass relabel and calculate blob features
  for (i = 0; i < rows; i++) 
  {
    for (j = 0; j < cols; j++) 
    {
      label_t &q = labels[i * cols + j]; 
      if (q.pixel > 0)
      {
        label_t &p = _find( q );
        if (!p.component) {
          assert(components_ct < MAXCOMPS); 
          p.component = &components[components_ct++]; 
          p.component->blob.bbox[0] = cols-1; 
          p.component->blob.bbox[1] = 0; 
          p.component->blob.bbox[2] = rows-1; 
          p.component->blob.bbox[3] = 0; 
          p.component->label = &p;
          p.component->blob.frame_width = cols; 
          p.component->blob.frame_height = rows; 
        }
        component_t &c = *p.component;
        c.blob.volume ++;
        c.blob.bbox[0] = min(c.blob.bbox[0], j); 
        c.blob.bbox[1] = max(c.blob.bbox[1], j); 
        c.blob.bbox[2] = min(c.blob.bbox[2], i); 
        c.blob.bbox[3] = max(c.blob.bbox[3], i);
        c.blob.centroid_x += i; 
        c.blob.centroid_y += j; 
        q.label = p.label;
        q.pixel = p.label % 255; /* FIXME */ 
      }
    }
  }

  // centroid calculation
  for (i = 0; i < components_ct; i++) {
    components[i].blob.centroid_x /= components[i].blob.volume; 
    components[i].blob.centroid_y /= components[i].blob.volume; 
  }
  
} // ccomp() 



void ConnectedComponents::_union (label_t &a, label_t &b)
{

  /* TODO This be done in O(1) if we maintain the rank of each node in the tree. 
   * Here we must traverse to the root for each union operation. */ 

  label_t *A = a.parent, *B = b.parent, *prev; 
  int rankA = 0, rankB = 0; 

  prev = &a; 
  while (A) {
    prev = A; 
    A = A->parent; 
    rankA ++; 
  }
  A = prev;
  
  prev = &b; 
  while (B) {
    prev = B; 
    B = B->parent; 
    rankB ++; 
  }
  B = prev;
  
  if (A != B)
  {
    if (rankA < rankB) 
      A->parent = B; 
  
    else
      B->parent = A; 
  }
} // _union()

ConnectedComponents::label_t &ConnectedComponents::_find (label_t &a) const
{
  label_t *A = a.parent, *prev; 
  prev = &a; 
  while (A) {
    prev = A;
    A = A->parent; 
  }
  A = prev; 
  return *A;  
} // _find()

int ConnectedComponents::getneighbors(label_t* neighbors [], int i, int j) 
{
  int ct = 0;

  //printf("(%d, %d) ", i, j); 
  for (int x = max(0, i-1); x <= min(i+1, rows-1); x++)
  {
    for (int y = max(0, j-1); y <= min(j+1, cols-1); y++)
    {
      if (labels[x * cols + y].label != UNASSIGNED) {
        //cout << labels[x * cols + y].label << ' '; 
        neighbors[ct++] = &labels[x * cols + y];
      }
    }
  }
  //cout << endl;
  
  return ct; 
} // getneighbors()

