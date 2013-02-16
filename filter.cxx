/* Christopher Patton
 * John Muir Institute for the Environment
 * University of California, Davis
 * June 2012
 * 
 * Input a list of filenames corresponding to a raw video stream. Output the 
 * names of files in which a target is captured. Apply binary threshold and
 * morphology filters. 
 * 
 */

#include "salamander.h"
#include "chunks.h"
#include "files.h"
#include <iostream>
#include <cstdlib>
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

param_t options; // threshold/morphology options 
char outname [256]; 
int  outname_index = 0; 

bool track( string &img ) 
{
  static int  i = 0; 
  static char outname [256];
  static ImageType::Pointer im;
  static vector<RelabelComponentImageFilterType::ObjectSizeType> sizes;

  im = threshold( img.c_str(), options );
  im = morphology( im, options );
  connectedComponents( im, sizes );
  sprintf(outname, "%s%d.jpg", options.prefix, outname_index++);
  write( im, outname ); 

  /* if there are blobs in the delta image, a target is in the frame */
  if( sizes.size() > 0 ) 
    return true;

  else return false;
}

int run( vector<string> &names ) 
{

  try 
  {
    for( int i = 0; i < names.size(); i++ ) {

      if( track( names[i] ) )
        cout << " b " << names[i] << endl;

      else 
        cout << "   " << names[i] << endl;
      
    }

  }
  
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught !" << std::endl;
    std::cerr << err << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS; 
}






int main(int argc, const char **argv) 
{
  int i, j;
      
  if (!parse_options( options, argc, argv ))
    die(help); 

  if (options.erode < 0) 
    die("error: must specify binary morphology factors");


  /* get file names */
  std::vector<std::string> names; 
  filenames( names, std::cin );
  
  return run( names ); 
}

