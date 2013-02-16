/* Christopher Patton
 * John Muir Institute for the Environment
 * University of California, Davis
 * June 2012
 * 
 *  TODO
 * 
 */

#include "salamander.h"
#include "chunks.h"
#include "files.h"
#include <iostream>
#include <cstdlib>
#include <assert.h>
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


bool delta( string &img1, string &img2, ImageType::Pointer &image, bool writeout=false ) 
{
  static ImageType::Pointer im;
  static vector<RelabelComponentImageFilterType::ObjectSizeType> sizes;

  im = delta( img1.c_str(), img2.c_str(), true, options );
  im = morphology( im, options  );
  connectedComponents( im, sizes );
  if (writeout) {
    sprintf(outname, "%s%d.jpg", options.prefix, outname_index++);
    write( im, outname ); 
  }

  /* if there are blobs in the delta image, a target is in the frame */
  if( sizes.size() > 0 ) {
    copy(image, im); // save a copy of the delta image as a side-effect
    return true;
  }

  else return false;
}


bool targetPersistsOverGap( vector<string> &names, int i, int j, const Blob &region )
{ 
  static ImageType::Pointer A, B, C; 
  static vector<RelabelComponentImageFilterType::ObjectSizeType> sizes;

  int sample = (i+j)/2; 
  A = read(names[i-1].c_str(), options); 
  B = read(names[i].c_str(), options); 
  C = read(names[sample].c_str(), options); 
  Blob target = region; 
  
  A = delta(A, C, target);
  A = threshold(A, options); 
  A = morphology(A, options);
  connectedComponents(A, sizes);
  bool a = (sizes.size() > 0); 
  sprintf(outname, "blob-%s-%s.jpg", names[i-1].c_str(), names[sample].c_str());
  write( A, outname ); 

  B = delta(B, C, target);
  B = threshold(B, options); 
  B = morphology(B, options);
  connectedComponents(B, sizes);
  bool b = (sizes.size() > 0); 
  sprintf(outname, "blob-%s-%s.jpg", names[i].c_str(), names[sample].c_str());
  write( B, outname ); 

  if (a && !b) 
    return true; 
  return false;  
}



int createChunks( vector<string> &names, Chunks &chunks ) 
/** 
 * Create a list of ranges of activity
 */ 
{
  try 
  {
    int master=0, left, right, s;
    Chunk *chunk=NULL, *prev=NULL; 
    ImageType::Pointer im; 
    
    /* Output images with target bounding box drawn. */
    bool tracking = false; 
    Blob lastSeen;
    char outname[128]; 
    
    for( int i = 1; i < names.size(); i++ ) {
		
      /* delta(i-1, i) */
      if( delta( names[i], names[i-1], im ) ) {

        cout << " * " << names[i] << endl;
        prev = chunks.back();
        chunk = new Chunk(); 
        if (prev)
          chunk->setStartPos( im, prev->getEndPos() ); 
        else 
          chunk->setStartPos( im ); 
        
        sprintf(outname, "tracking-%s", names[i].c_str());
        drawBoundingBox(names[i].c_str(), outname, 
                        chunk->getEndPos() * options.shrink_factor); 
        
        
        /* range where delta != 0. left is first appearance and 
         * right is when it disaappears */ 
        left = i; 
        for( i++ ; i < names.size() && delta( names[i], names[i-1], im ); i++ ) {
          cout << " | " << names[i] << endl;
          chunk->updateTarget( im ); 
          sprintf(outname, "tracking-%s", names[i].c_str());
          drawBoundingBox(names[i].c_str(), outname, 
                          chunk->getEndPos() * options.shrink_factor); 
        }
        right = --i; 
        chunk->setStartIndex( left ); 
        chunk->setEndIndex( right ); 
        
        chunks.append( chunk ); 
        if (prev) {
          if (targetPersistsOverGap(names, prev->getEndIndex(), chunk->getStartIndex(), prev->getEndPos()))
            chunks.mergeWithNext(prev); 
        }
        tracking = true; 
        lastSeen = chunk->getEndPos(); 


      }
      else {
        cout << "   " << names[i] << endl;
        if (tracking) {
          sprintf(outname, "tracking-%s", names[i].c_str());
          drawBoundingBox(names[i].c_str(), outname, 
                          lastSeen * options.shrink_factor); 
        }
      }
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



void printChunks( vector<string> &names, Chunks &chunks ) 
{
  int i = 0, j; 
  cout << "\n  Here are the blobs\n";
  for (Chunk *chunk = chunks.start(); chunk != NULL; chunk = chunks.next()) {
    cout << "\n chunk " << ++i << endl;
    cout << chunk->getStartPos() << endl;
    if (chunk->getEndIndex() - chunk->getStartIndex() == 0)
        cout << names[chunk->getEndIndex()] << endl;
    else if (chunk->getEndIndex() - chunk->getStartIndex() <= 6) 
      for (j = chunk->getStartIndex(); j < names.size() && j <= chunk->getEndIndex(); j++) {
        cout << names[j] << endl;
      }
    else {
        cout << names[chunk->getStartIndex()] << endl;
        cout << names[chunk->getStartIndex()+1] << endl;
        cout << names[chunk->getStartIndex()+2] << endl;
        cout << "   ...\n"; 
        cout << names[chunk->getEndIndex()-1] << endl;
        cout << names[chunk->getEndIndex()] << endl;
      }
      cout << chunk->getEndPos() << endl;
    }
  cout << endl;
}


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

  /* linked list of gaps */ 
  Chunks chunks; 
  createChunks( names, chunks ); 
  printChunks( names, chunks ); 

  return 0; 

}

