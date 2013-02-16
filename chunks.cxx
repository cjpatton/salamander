/* Christopher Patton
 * John Muir Institute for the Environment
 * University of California, Davis
 * Jan 2013
 * 
 * Data structures for representing video streams. We think of 
 * a video as a linked list consisting of chunks of activity (contiguous 
 * intervals where blobs appear in the delta image) separated by gaps
 * which may or may not contain a target. 
 */  

#include "salamander.h"
#include "chunks.h"
#include <iostream>

#define min(x,y) (x < y ? x : y)
#define max(x,y) (x < y ? y : x)

/** 
 * class Blob 
 */

Blob::Blob() {
  frameWidth = frameHeight = 0; 

  bbox[0] = 0;
  bbox[1] = 0;
  bbox[2] = 0;
  bbox[3] = 0;
  centroidX = centroidY = 0; 
  elongation = volume = 0; 
}

Blob::Blob( LabelGeometryImageFilterType::BoundingBoxType b,
            LabelGeometryImageFilterType::LabelPointType c, 
            double e, int v, 
            int w, int h )
{
  /* Create region from blob 
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
}

Blob Blob::operator*(int scale) const {
  Blob newBlob = *this;
  newBlob.frameHeight *= scale; 
  newBlob.frameWidth *= scale; 
  newBlob.bbox[0] = max(0, bbox[0]*scale); 
  newBlob.bbox[1] = min(newBlob.frameWidth-1, bbox[1]*scale); 
  newBlob.bbox[2] = max(0, bbox[2]*scale); 
  newBlob.bbox[3] = min(newBlob.frameHeight-1, bbox[3]*scale);  
  return newBlob; 
}
  
Blob& Blob::operator=(const Blob &blob) {
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
}

bool Blob::operator==(const Blob &blob) const {
/* Say two blobs are equivalent if they are roughly the same
 * size AND their intersect is at least 3/4 of their areas. */

  double aIntersect = (double)Intersects(blob),
         aRatio = aIntersect/GetBoundingBoxArea(),
         aBlobRatio = aIntersect/blob.GetBoundingBoxArea();

  //std::cout << aIntersect << " " << aRatio << " " << aBlobRatio << std::endl;

  if (abs(aRatio - aBlobRatio) < 0.25 && aRatio > 0.75 && aBlobRatio > 0.75)
    return true;

  else return false; 
}

bool Blob::operator!=(const Blob &blob) const {
  return !(*this == blob);
}

bool Blob::Contains(const Blob &blob) const {
  if (Intersects(blob) == blob.GetBoundingBoxArea())
    return true;
  else return false; 
}

int& Blob::operator[](int i) {
  return bbox[i]; 
}

int Blob::operator[](int i) const {
  return bbox[i]; 
}

void Blob::GetRegion( ImageType::RegionType& region ) const
{
  ImageType::IndexType index; 
  index[0] = bbox[0]; 
  index[1] = bbox[2]; 

  ImageType::SizeType size;
  size[0] = bbox[1] - bbox[0];
  size[1] = bbox[3] - bbox[2]; 

  region.SetIndex( index ); 
  region.SetSize( size );

}

int Blob::GetCentroidX() const {
  return centroidX; 
}

int Blob::GetCentroidY() const {
  return centroidY; 
}

double Blob::GetElongation () const {
  return elongation; 
}

int Blob::GetVolume() const {
  return volume; 
}

int Blob::GetBoundingBoxArea() const {
  return ((bbox[1]-bbox[0]) * (bbox[3]-bbox[2])); 
}

int Blob::DistanceTo( const Blob &blob ) const {
  return sqrt(pow(blob.centroidY - centroidY, 2) + 
                              pow(blob.centroidX - centroidX, 2));
}

int Blob::Intersects( const Blob &blob ) const {
  /* If bounding box is smaller than the sum of the heights and widths of the 
   * blobs, then they intersect. Assume the origin is in the top left corner.
   * Return area of intersect. Orientation of blob.boundingbox is: 
   * top left (0,2) (1,2)
   *        2.  (0,3) (1,3) bottom right */

  int width = max(bbox[1], blob.bbox[1]) - min(bbox[0], blob.bbox[0]); 
  int height = max(bbox[3], blob.bbox[3]) - min(bbox[2], blob.bbox[2]); 

  if (width < (bbox[1] - bbox[0] + blob.bbox[1] - blob.bbox[0]) && 
      height < (bbox[3] - bbox[2] + blob.bbox[3] - blob.bbox[2]))
    return (bbox[1] - bbox[0] + blob.bbox[1] - blob.bbox[0] - width) *  
           (bbox[3] - bbox[2] + blob.bbox[3] - blob.bbox[2] - height); 

  else return 0; 
  
}

void Blob::shiftOverMerged( const Blob &merged ) {
  
  int new_width = max(bbox[1]-bbox[0], bbox[3]-bbox[2]); 

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
  
  Blob smaller, bigger; 
  if (GetBoundingBoxArea() < merged.GetBoundingBoxArea()) {
    smaller = *this; 
    bigger = merged; 
  } else { 
    smaller = merged; 
    bigger =   *this; 
  }
  
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

  else { /*  */
    std::cout << "\t\t\tI dong it\n";
    *this = merged; /* I wonder if this will happen. */
  }

  bbox[0] = max(0, bbox[0]);
  bbox[1] = min(frameWidth, bbox[1]); 
  bbox[2] = max(0, bbox[2]); 
  bbox[3] = min(frameHeight, bbox[3]); 
}

bool Blob::containsPoint( int x, int y ) const {
  return (x >= bbox[0] && x <= bbox[1] && y >= bbox[2] && y <= bbox[3]);  
}

std::ostream& operator<< (std::ostream &out, const Blob &blob) 
{
  out //<< " centroid (" << blob.centroidX << ", " << blob.centroidY << ')'
      //<< " volume " << blob.volume << " elongation " << blob.elongation
      << " [" << blob.bbox[0] << ", " << blob.bbox[1] 
      << ", " << blob.bbox[2] << ", " << blob.bbox[3] << "]";
  return out; 
}



/**
 * class Chunk
 */

Chunk::Chunk( tuple<int> ch, Chunk *n ) {
  start_index = ch.x; 
  end_index = ch.y; 
  next = n; 
}

Chunk::Chunk() {
  next = NULL;
}

int Chunk::getStartIndex() const 
{ 
  return start_index; 
}

int Chunk::getEndIndex() const 
{ 
  return end_index; 
}

void Chunk::setStartIndex( int i ) 
{ 
  start_index = i; 
}

void Chunk::setEndIndex( int i ) 
{ 
  end_index = i; 
}

void Chunk::setStartPos( ImageType::Pointer &delta ) 
{
  /* for now, assume there is only ever one target to watch. */
  std::vector<Blob> blobs; 
  switch (getBlobs(delta, blobs)) {
    case 1: /* There should be only one blob in the delta frame
               at the start of a new chunk. (Of course, assuming
               the previous gap had no target.) */ 
      start_pos = end_pos = blobs[0]; 
      break;
               
    default: std::cout << "I Hope this doesn't happen yet(1)\n";    
  }
}

void Chunk::setStartPos( ImageType::Pointer delta, const Blob &last_known_pos ) 
{
  /* for now, assume there is only ever one target to watch. */
  std::vector<Blob> blobs; 
  switch (getBlobs(delta, blobs)) {
    case 2: /* If this is the case, then the blob that isn't 
               the same as end_pos should be the new end_pos. */
      if (last_known_pos.Intersects(blobs[0])) {
        start_pos = end_pos = blobs[1]; 
      }
      else {
        start_pos = end_pos = blobs[0]; 
      }
      std::cout << " moved(1) " <<  end_pos << std::endl;
      break;
    case 1: /* The target has moved, but not far enough for the 
               filtering step to produce result in two distinct 
               blobs. In this case, we need to determine the 
               direction in which the blob was moving, move it to 
               the new position, but keep the same dimensions as
               before. */ 
      if (!end_pos.Intersects(blobs[0])) { /* One blob, doesn't intersect with 
                                              previous. */ 
        start_pos = end_pos = blobs[0];     
        std::cout << " moved(2) " << end_pos << std::endl;
      } else if (end_pos == blobs[0]) { /* they are roughly equal in size and shape. 
                                         * In this case, the target has rotated or 
                                         * changed its orientation without moving
                                         * much. */
        start_pos = end_pos = blobs[0]; 
        std::cout << " approximately equal(1) " << end_pos << std::endl;
      } else {
        start_pos = last_known_pos;
        start_pos.shiftOverMerged(blobs[0]);  
        end_pos = start_pos; 
        std::cout << " shift over merged(1) " << end_pos << std::endl;
      }
      break;
    default: std::cout << "I Hope this doesn't happen yet(2)\n";    
  }

}



void Chunk::updateTarget( ImageType::Pointer &delta ) 
{
  /* for now, assume there is only ever one target to watch. */
  std::vector<Blob> blobs; 
  switch (getBlobs(delta, blobs)) {
    case 2: /* If this is the case, then the blob that isn't 
               the same as end_pos should be the new end_pos. */
      if (end_pos.Intersects(blobs[0])) {
        end_pos = blobs[1]; 
      }
      else 
        end_pos = blobs[0]; 
      std::cout << " moved(3) " <<  end_pos << std::endl;
      break;
    case 1: /* The target has moved, but not far enough for the 
               filtering step to produce result in two distinct 
               blobs. In this case, we need to determine the 
               direction in which the blob was moving, move it to 
               the new position, but keep the same dimensions as
               before. */ 
      if (!end_pos.Intersects(blobs[0])) { /* One blob, doesn't intersect with 
                                              previous. */ 
        end_pos = blobs[0];     
        std::cout << " moved(4) " << end_pos << std::endl;
      } else if (end_pos == blobs[0]) { /* they are roughly equal in size and shape. 
                                         * In this case, the target has rotated or 
                                         * changed its orientation without moving
                                         * much. */
        end_pos = blobs[0]; 
        std::cout << " approximately equal(2) " << end_pos << std::endl;
      } else {
        end_pos.shiftOverMerged(blobs[0]);  
        std::cout << " shift over merged(2) " <<  end_pos << std::endl;
      }
      break;
    default: std::cout << "I Hope this doesn't happen yet(2)\n";    
  }
} 

const Blob &Chunk::getStartPos() const 
{
  return start_pos; 
}

const Blob &Chunk::getEndPos() const
{
  return end_pos; 
} 


std::ostream &operator<<(std::ostream &out, const Chunk &chunk) {
  out << chunk.getStartPos() << std::endl;
  out << chunk.getEndPos() << std::endl;
  return out; 
}


/** 
 * class Chunks 
 */ 

Chunks::Chunks( ) {
  head = tail = NULL;  
  ct = 0; 
}

Chunks::~Chunks() {
  Chunk *tmp; 
  while (head != NULL) {
    tmp = head->next; 
    delete head; 
    head = tmp;
  }
}

int Chunks::size() const {
  return ct; 
}

Chunk *Chunks::start() {
  curr = head; 
  return curr; 
}

Chunk *Chunks::next() {
  if (curr) 
    curr = curr->next; 
  return curr;  
}

Chunk *Chunks::end() {
  return tail; 
}

Chunk *Chunks::append( Chunk *gap ) {
  ct ++;
  if (!head) {
    head = tail = gap; 
  } else {
    tail->next = gap; 
    tail = gap; 
  }
  return tail; 
}

Chunk *Chunks::back() {
  return tail;
}

void Chunks::mergeWithNext(Chunk *chunk) 
{
  if (tail == chunk->next) 
    tail = chunk;

  if (chunk->next) {
    Chunk *tmp = chunk->next; 
    chunk->end_index = chunk->next->end_index;
    chunk->end_pos =   chunk->next->end_pos;
    chunk->next =      chunk->next->next;
    delete tmp; 
  }

}

void Chunks::merge( int i, int j )
{
  /* Merge gaps into a chunk within range i to j */	
  Chunk *start = head, *end, *tmp; 
  while (start && start->getStartIndex() < i)
    start = start->next; 
  
  if (!start) 
    return;

  end = start->next;  
  while (end && end->getEndIndex() <= j) {
    start->setEndIndex( end->getEndIndex() );
    start->end_pos = end->end_pos; 
    tmp = end; 
    end = end->next; 
    delete tmp; 
  }

  start->next = end; 
      
}
