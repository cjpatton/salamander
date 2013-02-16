/* Christopher Patton
 * John Muir Institute for the Environment
 * University of California, Davis
 * June 2012
 * 
 * Provide statistics about blobs in order to determine a good threshold 
 * value for the filter. 
 * 
 * "calculate the derivative" of an image stream and apply binary morphology 
 * to the blobs in the delta image. Find the connected components and write out
 * the number of blobs per frame as well as the size of all blobs to file. 
 * 
 * 
 */

#include "salamander.h"
#include "chunks.h"
#include "files.h"
#include <cstdio> //sprintf()
using namespace std;

const char *help = 
"This is help for the Salamander project. Salamander is a set of tools for\n\
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

    ImageType::Pointer im;
    std::vector<RelabelComponentImageFilterType::ObjectSizeType> sizes;
    std::vector<Blob> blobs; 

    char outname[256]; 
    int outindex = 0; 

    try 
    {
        for( i = 1; i < names.size(); i ++ ) {
            cout << names[i-1] << ' ' << names[i] << endl;
            im = delta(names[i].c_str(), names[i-1].c_str(), true, options );
            im = morphology( im, options );
            getBlobs( im, blobs ); 

            sprintf(outname, "%s-%s-%s.jpg", options.prefix, names[i-1].c_str(), names[i].c_str()); 
            write( im, outname ); 

            for( j = 0; j < blobs.size(); j++) {
              cout << "     " << blobs[j] << endl; 
            }
        }
    }

    catch( itk::ExceptionObject & err )
    {
        std::cerr << "ExceptionObject caught !" << std::endl;
        std::cerr << err << std::endl;
        return EXIT_FAILURE;
    }
}





