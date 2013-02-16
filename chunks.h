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

#ifndef CHUNK_H
#define CHUNK_H
#include <vector>
#include <iostream>

/* Just a tuple. */ 
template <class T>
class tuple {
public:
  T x, y;
  tuple (T a, T b) {
    x = a;
    y = b;
  }
  void set(T a, T b) {
    x = a;
    y = b;
  }
  tuple() {
  }
};

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

/* Chunk is comprised of an index range of frames where the 
 * target is moving about. Also, calculate the start and 
 * ending position of the target (as a Blob). */ 
class Chunk {
friend std::ostream &operator<< (std::ostream &out, const Chunk &chunk);
friend class Chunks; 
public:

  Chunk( tuple<int> ch, Chunk *n=NULL );
  Chunk(); 
  int getStartIndex() const;
  int getEndIndex() const;
  void setStartIndex( int i );
  void setEndIndex( int i );  
  
  void setStartPos( ImageType::Pointer &delta );  // track target over chunk
  void setStartPos( ImageType::Pointer, const Blob &last_known_pos ); 
  void updateTarget( ImageType::Pointer &delta ); 
  const Blob &getStartPos() const; 
  const Blob &getEndPos() const; 

private: 

  int start_index, end_index;   // range where movement is detected
  Blob start_pos, end_pos;      // positions of target at the start and 
                                // end of chunk
  Chunk *next; 

};

std::ostream &operator<< (std::ostream &out, const Chunk &chunk);

/* Linked list of chunks and gaps. This is the main data structure for 
 * representing a video stream. */ 
class Chunks {
public:

  Chunks(); 
  ~Chunks();
  int size() const;
  Chunk *start(); 
  Chunk *next(); 
  Chunk *end(); 
  Chunk *append( Chunk *gap ); 
  Chunk *back();
  void merge( int i, int j );
  void mergeWithNext( Chunk *chunk ); 

private:
  
  Chunk *head, *curr, *tail;
  int ct; 

}; 

#endif
