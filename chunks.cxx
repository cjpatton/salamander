/* John Muir Institute for the Environment
 * University of California, Davis
 * 
 * chunks.cxx
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
 
 
#include "salamander.h"
#include "chunks.h"
#include <iostream>
#include <assert.h>


/**
 * class Chunk
 */

Chunk::Chunk( Chunk *p ) 
{
  prev = p; 
  next = NULL; 
  gap_known = false; 
} // constr

Chunk::Chunk() 
{
  next = NULL;
  prev = NULL; 
  gap_known = false; 
} // constr

bool Chunk::gapKnown() const 
{
  return gap_known; 
} // get gap_known

void Chunk::gapKnown( bool k ) 
{
  gap_known = k; 
} // set gap_known

int Chunk::getStartIndex() const 
{ 
  return start_index; 
} // get start_index

int Chunk::getEndIndex() const 
{ 
  return end_index; 
} // get end_index

void Chunk::setStartIndex( int i ) 
{ 
  start_index = i; 
} // set start_index

void Chunk::setEndIndex( int i ) 
{ 
  end_index = i; 
} // get end_index

void Chunk::setStartPos( ImageType::Pointer &delta, int i ) 
/* Set start position for track list. For now, assume there is only 
 * ever one target to watch. */
{
  std::vector<Blob> blobs; 
  switch (getBlobs(delta, blobs)) {
    case 1: /* There should be only one blob in the delta frame
               at the start of a new chunk. (Of course, assuming
               the previous gap had no target.) */ 
      tracks.push_back(Track(blobs[0], i)); 
      break;
               
    default: std::cout << "I Hope this doesn't happen yet(1)\n";    
  }
} // setStartPos() 

void Chunk::setStartPos( ImageType::Pointer delta, const Blob &last_known_pos, int i ) 
/* Set start position for track list given a known previous starting position. 
 * for now, assume there is only ever one target to watch. */
{
  std::vector<Blob> blobs; 
  switch (getBlobs(delta, blobs)) {
    case 2: /* If this is the case, then the blob that isn't 
               the same as end_pos should be the new end_pos. */
      if (last_known_pos.Intersects(blobs[0])) {
        tracks.push_back(Track(blobs[1], i)); 
      }
      else {
        tracks.push_back(Track(blobs[0], i));
      }
      std::cout << " moved(1) " << tracks.back().blob << std::endl;
      break;
    case 1: /* The target has moved, but not far enough for the 
               filtering step to produce result in two distinct 
               blobs. In this case, we need to determine the 
               direction in which the blob was moving, move it to 
               the new position, but keep the same dimensions as
               before. */ 
      if (!last_known_pos.Intersects(blobs[0])) { /* One blob, doesn't intersect with 
                                                     previous. */ 
        tracks.push_back(Track(blobs[0], i));
        std::cout << " moved(2) " << tracks.back().blob << std::endl;
      } else if (last_known_pos == blobs[0]) { /* they are roughly equal in size and shape. 
                                                  In this case, the target has rotated or 
                                                  changed its orientation without moving
                                                  much. */
        tracks.push_back(Track(blobs[0], i));
        std::cout << " approximately equal(1) " << tracks.back().blob << std::endl;
      } else {
        tracks.push_back(Track(last_known_pos, i)); 
        tracks.back().blob.shiftOverMerged(blobs[0]);      
        std::cout << " shift over merged(1) " << tracks.back().blob << std::endl;
      }
      break;
    default: std::cout << "I Hope this doesn't happen yet(2)\n";    
  }
} // setStartPos(lastKnown)

void Chunk::updateTarget( ImageType::Pointer &delta, int i ) 
/* Still processing the same chunk, update track list. For now, assume there 
 * is only ever one target to watch. */
{
  std::vector<Blob> blobs; 
  switch (getBlobs(delta, blobs)) {
    case 2: /* If this is the case, then the blob that isn't 
               the same as end_pos should be the new end_pos. */
      if (tracks.back().blob.Intersects(blobs[0])) {
        tracks.push_back(Track(blobs[1], i)); 
      }
      else {
        tracks.push_back(Track(blobs[0], i)); 
      }
      std::cout << " moved(3) " <<  tracks.back().blob << std::endl;
      break;
    case 1: /* The target has moved, but not far enough for the 
               filtering step to produce result in two distinct 
               blobs. In this case, we need to determine the 
               direction in which the blob was moving, move it to 
               the new position, but keep the same dimensions as
               before. */ 
      if (!tracks.back().blob.Intersects(blobs[0])) { /* One blob, doesn't intersect with 
                                                          previous. */ 
        tracks.push_back(Track(blobs[0], i));
        std::cout << " moved(4) " << tracks.back().blob << std::endl;
      } else if (tracks.back().blob == blobs[0]) { /* they are roughly equal in size and shape. 
                                                      In this case, the target has rotated or 
                                                      changed its orientation without moving
                                                      much. */
        tracks.push_back(Track(blobs[0], i)); 
        std::cout << " approximately equal(2) " << tracks.back().blob << std::endl;
      } else {
        tracks.push_back(Track(tracks.back().blob, i));
        tracks.back().blob.shiftOverMerged(blobs[0]); 
        std::cout << " shift over merged(2) " <<  tracks.back().blob << std::endl;
      }
      break;
    default: std::cout << "I Hope this doesn't happen yet(2)\n";    
  }
} // updateTarget()

const Blob &Chunk::getStartPos() const 
{
  return tracks[0].blob;
} // getStartPos()

const Blob &Chunk::getEndPos() const
{
  return tracks.back().blob; 
} // getEndPos()

const std::vector<Track> &Chunk::getTracks() const
{
  return tracks; 
} // getTracks()


std::ostream &operator<<(std::ostream &out, const Chunk &chunk) {
  out << chunk.getStartPos() << std::endl;
  out << chunk.getEndPos() << std::endl;
  return out; 
} // operator<< 



/** 
 * class Chunks 
 */ 

Chunks::Chunks( ) 
{
  head = tail = NULL;  
  ct = 0; 
} // constr

Chunks::~Chunks() 
{
  Chunk *tmp; 
  while (head != NULL) {
    tmp = head->next; 
    delete head; 
    head = tmp;
  }
} // destr

int Chunks::size() const 
{
  return ct; 
} // get ct 

Chunk *Chunks::start() 
/* head -> tail iterator */ 
{
  curr = head; 
  return curr; 
} // start() 

Chunk *Chunks::next() 
/* head -> tail iterator */ 
{
  if (curr) 
    curr = curr->next; 
  return curr;  
} // next() 

Chunk *Chunks::end() 
/* tail -> head iterator */ 
{
  curr = tail; 
  return curr; 
} // end() 

Chunk *Chunks::prev() {
/* tail -> head iterator */ 
  if (curr) 
    curr = curr->prev; 
  return curr; 
} // prev() 


Chunk *Chunks::append( Chunk *chunk ) 
/* Append a chunk to tail. This and mergeWithNext() are the only 
 * mutators for this object. */ 
{
  ct ++;
  if (!head) {
    head = tail = chunk; 
  } else {
    chunk->prev = tail; 
    tail->next = chunk; 
    tail = chunk; 
  }
  return tail; 
} // append() 

Chunk *Chunks::back() 
{
  return tail;
} // back() 

void Chunks::mergeWithNext(Chunk *chunk) 
/* Merge this chunk with the next one. This and append() are the only
 * mutators for this object. */ 
{
  if (chunk->next) {
    chunk->end_index = chunk->next->end_index; 
    const std::vector<Track> &tracks = chunk->next->tracks; 
    for (int i = 0; i < tracks.size(); i++)
      chunk->tracks.push_back(tracks[i]);    

    Chunk *del = chunk->next;
    if (del->prev && del->next) {
       del->prev->next = del->next;
       del->next->prev = del->prev;
    } else if (del->prev) {
       del->prev->next = del->next;
       tail = del->prev;
    } else if (del->next) {
       del->next->prev = del->prev;
       head = del->next;
    }
    delete del;
  }
} // mergeWithNext()
 
void Chunks::merge( int i, int j )
{
  /* TODO Merge gaps into a chunk within range i to j */	
}
