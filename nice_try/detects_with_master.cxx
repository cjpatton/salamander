/* Christopher Patton
 * John Muir Institute for the Environment
 * University of California, Davis
 * June 2012
 * 
 *  TODO
 * 
 */

#include "salamander.h"
#include <iostream>
#include <cstdlib>
using namespace std;

#define DIST 40

param_t options; // threshold/morphology options
char outname [256]; 
int  outname_index = 0; 

const char *help = " detects. \n\
\n\
  -t L H     Binary threshold range <L, H>. 0 < L < H < 255.\n\n\
  -m E D     Binary morphology erode and dilate factors. Eg., 2 20.\n\
             Threshold range defaults to <40, 60> if -t is unspecified.\n\n\
  -f name    File prefix for output files.\n\n\
  -h         Display this message.";


bool delta( string &img1, string &img2, bool writeout=true ) 
{
  static ImageType::Pointer im;
  static vector<RelabelComponentImageFilterType::ObjectSizeType> sizes;

  im = delta(img1.c_str(), img2.c_str(), true, options.low, options.high);
  im = morphology( im, options.erode, options.dilate );
  connectedComponents( im, sizes );
  sprintf(outname, "%s%d.jpg", options.prefix, outname_index++);
  if (writeout)
    write( im, outname ); 

  /* if there are blobs in the delta image, a target is in the frame */
  if( sizes.size() > 0 ) 
    return true;

  else return false;
}


ImageType::Pointer ddelta( string &img1, string &img2 ) 
{
  static ImageType::Pointer im;
  static vector<RelabelComponentImageFilterType::ObjectSizeType> sizes;

  im = delta(img1.c_str(), img2.c_str(), true, options.low, options.high);
  im = morphology( im, options.erode, options.dilate );
  sprintf(outname, "gap%d.jpg", outname_index++); 
  write( im, outname ); 
  return morphology( im, options.erode, options.dilate );
}


int createGaps( vector<string> &names, Gaps &gaps ) 
/** 
 * Create a list of ranges of activity
 * TODO I'm not sure that updating the master frame works properly. 
 */ 
{
  try 
  {
    int master=0, left, right, s;
    Gap *gap=NULL, *prev=NULL; 
    vector<Blob> blobs;
    for( int i = 1; i < names.size(); i++ ) {
		
      /* delta(i-1, i) */
      if( delta( names[i], names[i-1] ) ) {

        /* range where delta != 0. left is first appearance and 
         * left is when it disaappears */ 
        cout << " * " << names[i] << endl;
        left = i; 
        for( i++ ; i < names.size() && delta( names[i], names[i-1] ); i++ ) 
          cout << " | " << names[i] << endl;
        right = --i; 

        prev = gap; 
        gap = new Gap( tuple<int> (left, right) ); 
    
        /* Choose an image in the gap to compare to the master */     
        if (prev) {
          right = (prev->right() + gap->left()) / 2; 
        } else right = master; 
		
        /* Calculate delta(m,i) and make a list of blob centroids */
        ImageType::Pointer im = ddelta( names[right], names[master] ); 
        s = getBlobs( im, blobs ); 
        gap->SetBlobs( blobs ); 
        cout << " delta " << names[master] << ' ' << names[right] << " has "
             << s << " blobs\n"; 
        gaps.append( gap );   

        /* If there are no blobs in the delta(m,i), update master frame */
        if (s == 0) {
          master = right; 
        }

      }
      else cout << "   " << names[i] << endl;
    }

    /* Calculate gap blobs at the end of stream */ 
    right = (gaps.back()->right() + names.size()) / 2; 
    ImageType::Pointer im = ddelta(names[right], names[master]);
    getBlobs( im, blobs );
    gaps.SetEndGap( blobs ); 

  }
  
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught !" << std::endl;
    std::cerr << err << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS; 
}


int indexOfBlob( Blob &blob, vector<PersistentBlob> &pblobs ) 
{
  /* Return the index of the persistent blob within DIST of a blob.
   * Return -1 if blob is not persistent */
  for (int i = 0; i < pblobs.size(); i++) 
    if (pblobs[i].EuclideanDistanceTo( blob ) < DIST) {
      return i; 
    }
  return -1; 
}


void setMidPoint( Blob &a, const Blob &b ) 
{
  /* calculate midpoint of a and b and store as a. */ 
  vnl_vector_ref<double> x    = a.GetVnlVector(); 
  const vnl_vector<double> &y = b.GetVnlVector();
  x[0] = (x[0] + y[0]) / 2; 
  x[1] = (x[1] + y[1]) / 2; 
}




int mergeGaps( vector<string> &names, Gaps &gaps ) 
/**
 * Merge gaps with non-persisistent blobs. 
 */
{

  int index; 
  vector <PersistentBlob> pblobs;  
  Gap *prev = NULL;  
  for (Gap *gap = gaps.start(); gap != NULL; gap = gaps.next())
  {
    //cout << names[gap->left()] << " ... " << names[gap->right()] << endl; 
    vector<Blob> &blobs = gap->GetBlobs(); 

    for (int i = 0; i < blobs.size(); i++) {
      index = indexOfBlob( blobs[i], pblobs );
      if (index < 0) { /* blob first appears */ 
        if (prev) 
          pblobs.push_back( PersistentBlob(blobs[i], prev->left(), gap->right()) );
        else 
          pblobs.push_back( PersistentBlob(blobs[i]) );          
      } else { /* blob persistent */ 
        setMidPoint( pblobs[index], blobs[i] );
        pblobs[index].right( gap->right() ); 
      }
    }

    for (int i = 0; i < pblobs.size(); i++) {
      if (pblobs[i].right() < gap->right()) { /* blob ends! merge range */ 
        gaps.merge(pblobs[i].left(), pblobs[i].right()); 
      }
    }
    prev = gap; 
  }
  
  /* end gap */ 
  vector<Blob> &blobs = gaps.GetEndGap(); 
  for (int i = 0; i < blobs.size(); i++) {
    index = indexOfBlob( blobs[i], pblobs );
    if (index >= 0) {
      setMidPoint( pblobs[index], blobs[i] );
      pblobs[index].right( names.size() ); 
    }
  }

  for (int i = 0; i < pblobs.size(); i++) {
    if (pblobs[i].right() < names.size()) { /* blob ends! merge range */ 
      gaps.merge(pblobs[i].left(), pblobs[i].right()); 
    }
  }

  return 0; 
}

void printChunks( vector<string> &names, Gaps &gaps ) 
{
  int i = 0, j; 
  for (Gap *gap = gaps.start(); gap != NULL; gap = gaps.next()) {
    cout << "\n chunk " << ++i << endl;
    if (gap->right() - gap->left() == 0)
      cout << names[gap->right()] << endl;
    else if (gap->right() - gap->left() <= 6) 
      for (j = gap->left(); j < gap->right(); j++) {
        cout << names[j] << endl;
      }
    else {
      cout << names[gap->left()] << endl;
      cout << names[gap->left()+1] << endl;
      cout << names[gap->left()+2] << endl;
      cout << "   ...\n"; 
      cout << names[gap->right()-1] << endl;
      cout << names[gap->right()] << endl;
    }
  }
  cout << endl;
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

  /* linked list of gaps */ 
  Gaps gaps; 
  createGaps( names, gaps ); 
  mergeGaps( names, gaps ); 
  printChunks( names, gaps ); 

  return 0; 

}

