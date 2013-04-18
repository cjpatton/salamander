/* John Muir Institute for the Environment
 * University of California, Davis
 * 
 * filter.cpp
 * Top-level program applies image processing pipeline to a stream of 
 * files. Output a list of blobs for each delta. This file is part of the 
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
#include "files.h"
#include <cstdio> //sprintf()
#include <iostream>
using namespace std;

const char *help = 
" flter - apply image procesing pipeline to video stream.\n\
This is help for the Salamander project. Salamander is a set of tools for\n\
automated filtering of video streams for targets of interest. These programs\n\
input a list of JPEG images on standard input and process them in alphanumeric\n\
order, e.g., ls *.jpg | detect <options>\n\
\n\
  -t L H     Binary threshold range <L, H>. 0 < L < H < 255.\n\n\
  -m E D     Binary morphology erode and dilate factors. Eg., 2 20.\n\
             Threshold range defaults to <40, 60> if -t is unspecified.\n\n\
  -s N       Image shrink factor. Defaults to 1 (don't shrink)\n\n\
  -f name    File prefix for output files.\n\n\
  -h         Display this message.";

int main(int argc, const char **argv) 
{
    int i, j; 

    param_t options;

    if (!parse_options( options, argc, argv ))
      die(help);

    if (options.erode < 0) 
      die("error: must specify binary morphology factors");

    /* get file names */
    std::vector<std::string> names; 
    filenames( names, std::cin );

    cv::Mat im;
    std::vector<Blob> blobs; 

    char outname[256]; 
    int outindex = 0; 

    try 
    {
        for( i = 1; i < names.size(); i ++ ) {
            cout << names[i-1] << ' ' << names[i] << endl;
            delta(im, names[i].c_str(), names[i-1].c_str(), true, options );
            morphology( im, options );
            getBlobs( im, blobs ); 

            sprintf(outname, "%s-%s-%s.jpg", options.prefix, names[i-1].c_str(), names[i].c_str()); 
            cv::imwrite( outname, im ); 

            for( j = 0; j < blobs.size(); j++) {
              cout << "     " << blobs[j] << endl; 
            }
        }
    }

    catch( cv::Exception &e )
    {
        std::cerr << "-- Exception ------\n" 
                  << e.what() << std::endl
                  << "-------------------\n";
        return EXIT_FAILURE;
    }
}
