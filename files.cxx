/* John Muir Institute for the Environment
 * University of California, Davis
 * 
 * files.cxx
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

#include "files.h"
#include <limits> 
#include <vector>
#include <algorithm> //sort()
#include <string> 
#include <iostream>
#include <stdio.h>
#include <string.h> 
#include <errno.h>
#include <sys/stat.h>
#include <cmath>

#define NUMERIC(x) (x >= '0') && (x <= '9') 
#define ALPHABETIC(x) ((x >= 'a') && (x <= 'z')) || ((x >= 'A') && (x <= 'Z'))

void die( const char *msg )
{
   std::cerr << msg << std::endl;
   exit(EXIT_FAILURE); 
} // die() 

bool empty( const char *file ) 
{
  static struct stat st;
  stat(file, &st);
  switch( errno ) {
    case ENOENT:
      std::cerr << "can't stat " << file << ": file not found\n" << std::endl;
      return true;
  }
  return (st.st_size <= 0);
} // empty() 

void filenames( std::vector<std::string> &names, std::istream &in ) 
/* Get filenames from standard input and filter out corrupted ones. So far, 
 * these are images that are empty (0 bytes). Sort files in alphanumeric 
 * order. */
{
  char name [128]; 
  in.getline(name, 128);
  while( name[0] != '\0' ) 
  {
    if( !empty(name) )
      names.push_back( std::string(name) ); 
    in.getline(name, 128);
  }
  sort(names.begin(), names.end(), cmp);
} // filenames()


bool cmp(const std::string &a, const std::string &b) 
/* compare function for sorting filenames Compare alphabetically and 
 * numerically. Use this way: for files of type vector<string>, 
 * sort(files.begin(), files.end(), cmp). */
{
  unsigned i=0, j=0, m, n, an, bn; 
  while( i < a.size() && j < b.size() ) {

    if (NUMERIC(a[i]) && NUMERIC(b[j])) {

      m = i;
      while( m < a.size() && NUMERIC(a[m]) )
        m++; 

      n = j; 
      while( n < b.size() && NUMERIC(a[n]) )
        n++;

      an = atoi( a.substr(i,m).c_str() ); 
      bn = atoi( b.substr(j,n).c_str() );

      if (an < bn) return true; 
      else if (an > bn) return false;

      i = m; 
      j = n; 

    }

    if (a[i] < b[j])
      return true;
    else if (a[i] > b[j]) 
      return false; 

    i++; 
    j++;
  }

  return a.size() < b.size();
} // cmp()

void sample(std::vector<int> &samples, int ct, int i, int j, int mean, int sd) 
/* Box-Muller method for approximating a normal distribution. Generate normally 
 * distributed samples over a range of indices. These indicies reference a 
 * list of filenames stored in an std::vector. */ 
{
  samples.clear(); 
  int tmp; 
  if (j < i) {
    tmp = i;
    i = j; 
    j = tmp; 
  }

  double U, V, X, Y, pi = (double)22/7; 
  
  tmp = ct; 
  while (tmp > 0) {
    /* uniform random number in (0,1] */
    U = ((double)(rand() % 100000) / 100000);
    V = ((double)(rand() % 100000) / 100000);

    /* X and Y are independent random variables drawn from a normal 
     * distribution. Default is X,Y~N(1,0), or standard normal Z. */ 
    U = sqrt(-2 * log(U)); 
    X = U * cos(2 * pi * V);
    Y = U * sin(2 * pi * V); 
    X = (X * sd) + mean; 
    Y = (Y * sd) + mean; 

    /* Throw out samples outside of range */
    if (i < (int)X && (int)X < j) {
      samples.push_back((int)X); 
      tmp--;  
    } 

    if (tmp > 0 && i < (int)Y && (int)Y < j) {
      samples.push_back((int)Y); 
      tmp--; 
    }
  }
} // sample()



int parse_options( param_t &options, int argc, const char **argv ) 
/* Parse command line options and return a status, informing the up stream 
 * if the parameters weren't inputted correctly. */ 
{
  options.shrink_factor = options.low = options.high = options.erode = options.dilate = -1; 
  options.prefix[0] = '\0';

  for (int i = 1; i < argc; i++) 
  {
  
    /* binary threshold */
    if (strcmp(argv[i], "-t") == 0 && (argc - i) > 2) { 
      if (!NUMERIC(argv[i+1][0]) || !NUMERIC(argv[i+2][0])) 
        return 0; 
      options.low = atoi(argv[++i]); 
      options.high = atoi(argv[++i]); 
      if (options.low < 0 || options.low > options.high || options.high > 255)
        return 0; 
    } 
    
    /* binary morpholoy */ 
    else if (strcmp(argv[i], "-m") == 0 && (argc - i) > 2) { 
      if (!NUMERIC(argv[i+1][0]) || !NUMERIC(argv[i+2][0])) 
        return 0; 
      options.erode = atoi(argv[++i]); 
      options.dilate = atoi(argv[++i]); 
      if (options.erode < 0 || options.dilate < 0)
        return 0; 
      if (options.low == -1) {
        options.low = 40; 
        options.high = 60;
      }
    }
    
    /* output file prefix */ 
    else if (strcmp(argv[i], "-f") == 0 && (argc - i) > 1) { 
      i++; 
      for (int j = 0; j < strlen(argv[i]); j++) {
        if (!ALPHABETIC(argv[i][j])) 
          return 0;
      }
      strcpy(options.prefix, argv[i]); 
    }
    
    /* shrink factor */ 
    else if (strcmp(argv[i], "-s") == 0 && (argc - i) > 1) { 
      options.shrink_factor = atoi(argv[++i]);
      if (options.shrink_factor < 1)
        return 0; 
    }
    else 
      return 0; 
    
  }
  
  if (options.shrink_factor < 0) 
    options.shrink_factor = 1; 

  if (options.prefix[0] == '\0')
    strcpy(options.prefix, "test"); 

  return 1; 

} // parse_options()
