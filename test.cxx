/* Christopher Patton
 * John Muir Institute for the Environment
 * University of California, Davis
 *
 * 
 * This gives a nice distribution for sampling over gaps 
 *
 */


#include "salamander.h"
#include "chunks.h"
#include "files.h"
#include <iostream>
using namespace std; 

int main(int argc, const char **argv) 
{
  if (argc != 5) {
    die("usage: test <bounding box>");
  }

  Blob box; 
  box[0] = atoi(argv[1]); 
  box[1] = atoi(argv[2]); 
  box[2] = atoi(argv[3]); 
  box[3] = atoi(argv[4]); 
    
  cout << box * 3 << endl;
  
  
  
  return 0; 
}


