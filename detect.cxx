/* John Muir Institute for the Environment
 * University of California, Davis
 * 
 * detect.cxx TODO 
 * Here is where I will work on a more sophisticated approach to 
 * detection. Currently, detetion assumes that all noise is filtered 
 * out in binary thersholding and morphology. While this is good for 
 * small targets and a high frequency of images, it is less practical
 * when noise is an inevitability. The data I'll be working with will 
 * field cameras capturing deer footage. A photo is taken every 4 
 * minutes. 
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
#include "files.h"
#include <iostream>
#include <cstdlib>
#include <assert.h>
using namespace std;

#define SWAP(x,y) { \
   (x) ^= (y);      \
   (y) ^= (x);      \
   (x) ^= (y);      \
}

const char *help = 
" detect[NOT IMPLEMENTED] - more sophisticated detection scheme.\n\
This is help for the Salamander project. Salamander is a set of tools for\n\
automated filtering of video streams for targets of interest. These programs\n\
input a list of JPEG images on standard input and process them in alphanumeric\n\
order. Eg. ls *.jpg | detect <options>\n\
\n\
  -t L H     Binary threshold range <L, H>. 0 < L < H < 255.\n\n\
  -m E D     Binary morphology erode and dilate factors. Eg., 2 20.\n\
             Threshold range defaults to <40, 60> if -t is unspecified.\n\n\
  -s N       Image shrink factor. Defaults to 1 (don't shrink)\n\n\
  -f name    File prefix for output files.\n\n\
  -h         Display this message.";

param_t options; // threshold/morphology options
char outname [256]; 
int  outname_index = 0; 

int main(int argc, const char **argv) 
{

  srand (time(NULL)); 

  if (!parse_options( options, argc, argv ))
    die(help);

  if (options.erode < 0) 
    die("error: must specify binary morphology factors");

  /* get file names */
  std::vector<std::string> names; 
  filenames( names, std::cin );

  /* TODO - the actual work. */ 

  return 0; 
}
