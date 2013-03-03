/* John Muir Institute for the Environment
 * University of California, Davis
 * 
 * blob.cxx
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

#include "salamander.h"
#include "blob.h"

#define min(x,y) (x < y ? x : y)
#define max(x,y) (x < y ? y : x)

bool Blob::containsPoint( int x, int y ) const 
/* Bounding box contains point (x,y) */
{
  return (x >= bbox[0] && x <= bbox[1] && y >= bbox[2] && y <= bbox[3]);  
} // containsPoint() 



Blob::Blob() {
  frameWidth = frameHeight = 0; 
  bbox[0] = 0;
  bbox[1] = 0;
  bbox[2] = 0;
  bbox[3] = 0;
  centroidX = centroidY = 0; 
  elongation = volume = 0; 
} // dumb constr

Blob::Blob( LabelGeometryImageFilterType::BoundingBoxType b,
            LabelGeometryImageFilterType::LabelPointType c, 
            double e, int v, 
            int w, int h )
{
  /* bounding box corners: 
   * (0,2) (1,2)
   * (0,3) (1,3) */

  frameWidth = w; 
  frameHeight = h; 

  bbox[0] = b[0];
  bbox[1] = b[1];
  bbox[2] = b[2];
  bbox[3] = b[3];

  centroidX = c[0]; 
  centroidY = c[1];

  elongation = e; 
  volume = v;    
} // salamander.cxx::getBlobs() constr

Blob::Blob( int top, int right, int bottom, int left ) 
{
  frameWidth = frameHeight = 0; 
  bbox[0] = top;
  bbox[1] = right;
  bbox[2] = bottom;
  bbox[3] = left;
  
  centroidX = centroidY = 0; 
  elongation = volume = 0; 
} // bounding box constr


Blob Blob::operator*(int scale) const 
/* Images can be scaled in order to improve efficiency. This operator 
 * is useful for rescaling blobs the original image. Say a scaling factor
 * of 3 is used. Then salamander.cxx::drawBoundingBox() should be used 
 * like drawBoundingBox(original_image, target_blob * 3). */ 
{
  Blob newBlob = *this;
  newBlob.frameHeight *= scale; 
  newBlob.frameWidth *= scale; 
  newBlob.bbox[0] = max(0, bbox[0]*scale); 
  newBlob.bbox[1] = min(newBlob.frameWidth-1, bbox[1]*scale); 
  newBlob.bbox[2] = max(0, bbox[2]*scale); 
  newBlob.bbox[3] = min(newBlob.frameHeight-1, bbox[3]*scale);  
  return newBlob; 
} // operator* 
  
Blob& Blob::operator=(const Blob &blob) 
{
  frameWidth = blob.frameWidth; 
  frameHeight = blob.frameHeight; 

  bbox[0] = blob.bbox[0];
  bbox[1] = blob.bbox[1];
  bbox[2] = blob.bbox[2];
  bbox[3] = blob.bbox[3];

  centroidX = blob.centroidX; 
  centroidY = blob.centroidY;

  elongation = blob.elongation; 

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
  bbox[1] = min(frameWidth, bbox[1]); 
  bbox[2] = max(0, bbox[2]); 
  bbox[3] = min(frameHeight, bbox[3]); 
  
} // shiftOverMerged()

int Blob::GetBoundingBoxArea() const 
{
  return ((bbox[1]-bbox[0]) * (bbox[3]-bbox[2])); 
} // GetBoundingBoxArea()



void Blob::GetRegion( ImageType::RegionType& region ) const
/* Create ITK region from bounding box */ 
{
  ImageType::IndexType index; 
  index[0] = bbox[0]; 
  index[1] = bbox[2]; 

  ImageType::SizeType size;
  size[0] = bbox[1] - bbox[0];
  size[1] = bbox[3] - bbox[2]; 

  region.SetIndex( index ); 
  region.SetSize( size );
} // GetRegion()



int Blob::DistanceTo( const Blob &blob ) const 
/* Euclidean distance between blob centroids */ 
{
  return sqrt(pow(blob.centroidY - centroidY, 2) + 
                              pow(blob.centroidX - centroidX, 2));
} // DistanceTo() 

int Blob::GetCentroidX() const 
{
  return centroidX; 
} // GetCentroidX()

int Blob::GetCentroidY() const 
{
  return centroidY; 
} // GetCentroidY() 

double Blob::GetElongation () const 
{
  return elongation; 
} // GetElongation() 

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
