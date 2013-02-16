/* Christopher Patton
 * John Muir Institute for the Environment
 * University of California, Davis
 * June 2012
 * 
 * Program for experimenting with binary morphology.
 * 
 */



#include "salamander.h"
#include "files.h"

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
    ImageType::Pointer im;        

    try 
    {
        for( i = 1; i < names.size(); i ++ ) {
            sprintf(outname, "binmorph%d.jpg", i);
            im = delta(names[i-1].c_str(), names[i].c_str(), true, options);
            im = morphology( im, options );
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

