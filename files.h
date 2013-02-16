/* John Muir Institute for the Environment
 * University of California, Davis
 * 
 * files.h
 * Routines and data structures for handling input file names and command
 * line parameters. This file is part of the Salamander project. 
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


#ifndef FILES_H
#define FILES_H
#include <vector>
#include <string>

void die( const char *msg );

/**
 * Check if file is 0 bytes
 */
bool empty( const char *file );

/**
 * Get filenames from standard input
 */
void filenames( std::vector<std::string> &names, std::istream &in );

/**
 * Sort function for files
 */ 
bool cmp(const std::string &a, const std::string &b); 


/** 
 * Command line options 
 */ 
struct param_t { 
  
  int erode, dilate; // bianry morphology factors
  int low,     high; // binary threshold range
  int shrink_factor; // shrink image for efficiency
  char prefix [256]; 

}; 

int parse_options( param_t &options, int argc, const char **argv ); 


#endif
