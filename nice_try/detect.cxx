/* Christopher Patton
 * John Muir Institute for the Environment
 * University of California, Davis
 * June 2012
 * 
 * Input a list of filenames corresponding to a raw video stream. Output the 
 * names of files in which a target is captured. 
 * 
 * "calculate the derivative" of an image stream and apply binary morphology 
 * to the blobs in the delta image. Find the connected components and write out
 * the number of blobs per frame as well as the size of all blobs to file. 
 * 
 * 
 */

#include "salamander.h"
#include <iostream>
#include <cstdlib>
using namespace std;



param_t options; // threshold/morphology options
char outname [256]; 
int  outname_index = 0; 

bool delta( string &img1, string &img2, bool writeout=true ) 
{
  static ImageType::Pointer im;
  static vector<RelabelComponentImageFilterType::ObjectSizeType> sizes;

  im = delta(img1.c_str(), img2.c_str(), true, options);
  im = morphology( im, options );
  connectedComponents( im, sizes );
  sprintf(outname, "%s%d.jpg", options.prefix, outname_index++);
  if (writeout)
    write( im, outname ); 

  /* if there are blobs in the delta image, a target is in the frame */
  if( sizes.size() > 0 ) 
    return true;

  else return false;
}

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


int merge( vector<tuple<int> > &detects, int i, int j )
/* Merge Ranges */
{
  if (i >= j)
    return 0; 
  detects[i].right = detects[j].right;
  detects.erase( detects.begin()+(++i), detects.begin()+(++j) );
  return 1; 
}

void chunks( vector<string> &names, vector<tuple<int> > &detects )
{
  if (detects.size() > 0) {
    cout << " chunks " << names[detects[0].left] << ' ' << names[detects[0].right] << endl;
    for (int i = 1; i < detects.size(); i++)
      cout << "        " << names[detects[i].left] << ' ' << names[detects[i].right] << endl;
  }
}


int run( vector<string> &names, vector<tuple<int> > &detects ) 
{

  try 
  {
    int left, right, s; 
    for( int i = 1; i < names.size(); i++ ) {
    
      if( delta( names[i], names[i-1] ) ) {

        /* range where delta != 0. left is first appearance
         * and left is when it disaappears */ 
        
        cout << " * " << names[i] << endl;
        left = i; 
    
        for( i++ ; i < names.size() && delta( names[i], names[i-1] ); i++ ) 
          cout << " | " << names[i] << endl;
        
        right = --i; 

        detects.push_back( tuple<int> (left, right) ); 

        /* merge previous two ranges if stationary target 
         * between them. */

        if ((s = detects.size()) > 1) {
          left = (s > 2 ? 
             (detects[s-2].left / 2) : 
             (detects[s-3].right + detects[s-2].left) / 2);
      
          right = (detects[s-2].right + detects[s-1].left) / 2; ; 
          if( delta( names[right], names[left] )) {
            merge( detects, detects.size()-2, detects.size()-1 );
            cout << " Merge! " << names[left] << ' ' << names[right] << endl;
          }
          else
            cout << "        " << names[left] << ' ' << names[right] << endl;
            
          chunks(names, detects); 
        }
          
      }

      else cout << "   " << names[i] << endl;

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
  if (!parse_options( options, argc, argv ))
    die(help);

  if (options.erode < 0) 
    die("error: must specify binary morphology factors");

  /* get file names */
  std::vector<std::string> names; 
  filenames( names, std::cin );

  std::vector<tuple<int > > detects; 
  
  int res = run( names, detects ); 

  for (int i = 0; i < detects.size(); i++) {
    cout << names[detects[i].left] << " ... " << names[detects[i].right]
         << " for " << detects[i].right - detects[i].left << " fraems\n"; 
  }

  return res; 

}

