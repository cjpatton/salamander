/* John Muir Institute for the Environment
 * University of California, Davis
 * 
 * binary_threshold.cxx
 * Apply binary threshold to a series of images. This file is part of 
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
#include <iostream>

int main(int argc, const char **argv) 
{

    bool threshold = false;  
    int low, high, i; 
    
    
    if (argc == 3) {
        threshold = true;
        low = atoi(argv[1]); high = atoi(argv[2]); 
        if (low > high) {
            i = low; 
            low = high;
            high = i; 
        }
    } else {
        // don't threshold
    }
    
    param_t options; 
    options.low = low; 
    options.high = high; 
    options.shrink_factor = 1; 

    /* get file names */
    std::vector<std::string> names; 
    filenames( names, std::cin );
    char outname [256];
        
    try 
    {
        for( i = 1; i < names.size(); i ++ ) {
            sprintf(outname, "binthresh%d.jpg", i);
            cv::Mat im;
            delta(im, names[i-1].c_str(), names[i].c_str(), threshold, options);
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

