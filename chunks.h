/* John Muir Institute for the Environment
 * University of California, Davis
 * 
 * chunks.h
 * Data structures for representing/segmenting a video stream and tracking
 * targets across frames. This file is part of the Salamander project. 
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


#ifndef CHUNK_H
#define CHUNK_H
#include <vector>
#include <iostream>
#include "blob.h"

struct Track {
  Blob blob; 
  int  index; 

  Track() {
  }

  Track(const Blob &b, int i) {
    blob =  b; 
    index = i; 
  }
}; 



/* Chunk is comprised of an index range of frames where the 
 * target is moving about. Also, calculate the start and 
 * ending position of the target (as a Blob). */ 
class Chunk {
friend std::ostream &operator<< (std::ostream &out, const Chunk &chunk);
friend class Chunks; 
public:

  Chunk( Chunk *p );
  Chunk(); 
  bool gapKnown() const; 
  void gapKnown( bool k ); 
  int getStartIndex() const;
  int getEndIndex() const;
  void setStartIndex( int i );
  void setEndIndex( int i );
  
  void setStartPos( ImageType::Pointer &delta, int i );  // track target over chunk
  void setStartPos( ImageType::Pointer, const Blob &last_known_pos, int i ); 
  void updateTarget( ImageType::Pointer &delta, int i ); 
  const Blob &getStartPos() const; 
  const Blob &getEndPos() const; 
  const std::vector<Track>& getTracks() const; 

private: 

  bool gap_known;               // is preceeding gap known to not contain a target
  int start_index, end_index;   // range where movement is detected
  std::vector<Track> tracks; 
  Chunk *prev, *next; 

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
  Chunk *prev();  
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
