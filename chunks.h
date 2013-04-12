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


class TrackException {
  friend std::ostream &operator<<(std::ostream &out, const TrackException &err); 
  char msg [128]; 

  public:
    TrackException(const char *str); 
};

std::ostream &operator<<(std::ostream &out, const TrackException &err);



/** 
 * struct Track - a (Blob, index) pair. This signifies that a target 
 * appeared in the location given by the Blob at the time index. Chunks 
 * store a list of these. 
 */

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



/** 
 * class Chunk - a section of video in which a target is known to be
 * present. Store its position w.r.t. in a (Blob, index) list. If a 
 * two adjacent chunks are known to have a target between them, their
 * track lists will be merged. 
 */

class Chunk {
friend std::ostream &operator<< (std::ostream &out, const Chunk &chunk);
friend class Chunks; 
public:

  Chunk( Chunk *p );
  Chunk(); 

  /* Preceeding gap is known to NOT contain a target. */ 
  bool gapKnown() const; 
  void gapKnown( bool k ); 

  /* Time index range of chunk. */ 
  int getStartIndex() const;
  int getEndIndex() const;
  void setStartIndex( int i );
  void setEndIndex( int i );
  
  /* Routines for target tracking. */ 
  void setStartPos( const cv::Mat &delta, int i );  
  void setStartPos( const cv::Mat&, const Blob &last_known_pos, int i ); 
  void updateTarget( const cv::Mat &delta, int i ); 
  const Blob &getStartPos() const; 
  const Blob &getEndPos() const; 
  const std::vector<Track>& getTracks() const; 

private: 

  bool gap_known;               /* preceeding gap known to NOT contain a target */ 
  int start_index, end_index;   /* time index of range of chunk */ 
  std::vector<Track> tracks;    /* (Blob, index) list */ 
  Chunk *prev, *next;  

};

std::ostream&operator<< (std::ostream &out, const Chunk &chunk);



/**
 * class Chunks - a double linked list that represents segments in a video
 * feed in which a target appears. 
 */ 

class Chunks {
public:

  Chunks(); 
  ~Chunks();
  int size() const;

  /* iterators, forward and backward */ 
  Chunk *start(); 
  Chunk *next();
  Chunk *prev();  
  Chunk *end(); 
  Chunk *append( Chunk *gap ); 
  Chunk *back();

  /* Gap between chunks known to contain a target, merge them. */ 
  void mergeWithNext( Chunk *chunk ); 

  /* Merge an entire range. */ 
  void merge( int i, int j ); // TODO 

private:
  
  Chunk *head, *curr, *tail;
  int ct; 

}; 

#endif
