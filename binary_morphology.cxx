/* John Muir Institute for the Environment
 * University of California, Davis
 * 
 * binary_morphology.cxx
 * Apply binary morphology to a series of images. This file is part of 
 * the Salamander project. 
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
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "highgui.h"

#include <iostream>

int main(int argc, const char **argv) 
{

    int low, high;
    int erode_factor, dilate_factor;
    int i; 
    
    
    if (argc >= 3) {
        erode_factor = atoi(argv[1]); 
        dilate_factor = atoi(argv[2]); 
        if (argc >= 5) {
            low = atoi(argv[3]); high = atoi(argv[4]); 
            if (low > high) {
                i = low; 
                low = high;
                high = i; 
            }
        } else {
            low = 40;    // best values found by experiment
            high = 60; 
        }  
    } else { 
        die("usage: binmorph erode_factor dilate_factor [threshold range]"); 
    }

    param_t options; 
    options.low = low; 
    options.high = high; 
    options.erode = erode_factor; 
    options.dilate = dilate_factor;
    options.shrink_factor = 1; 

    /* get file names */
    std::vector<std::string> names; 
    filenames( names, std::cin );
    char outname [256];
    cv::Mat im;        

    try 
    {
        for( i = 1; i < names.size(); i ++ ) {
            sprintf(outname, "binmorph%d.jpg", i);
            delta(im, names[i-1].c_str(), names[i].c_str(), true, options);
            morphology( im, options );
            write( im, outname );
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

