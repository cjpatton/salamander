/* Christopher Patton
 * John Muir Institute for the Environment
 * University of California, Davis
 * June 2012
 * 
 * Program for experimenting with binary thresholding.
 * 
 */


#include "salamander.h"
#include "files.h"

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
            ImageType::Pointer im = delta(names[i-1].c_str(), names[i].c_str(), threshold, options);
            write( im, outname );
        }
    }

    catch( itk::ExceptionObject & err )
    {
        std::cerr << "ExceptionObject caught !" << std::endl;
        std::cerr << err << std::endl;
        return EXIT_FAILURE;
    }
}

