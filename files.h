











#ifndef FILES_H
#define FILES_H
#include <vector>
#include <string>

void die( const char *msg );

/**
 * Check if file is 0 bytes
 */
bool empty( const char *file );

/**
 * Get filenames from standard input
 */
void filenames( std::vector<std::string> &names, std::istream &in );

/**
 * Sort function for files
 */ 
bool cmp(const std::string &a, const std::string &b); 


/** 
 * Command line options 
 */ 
struct param_t { 
  
  int erode, dilate; // bianry morphology factors
  int low,     high; // binary threshold range
  int shrink_factor; // shrink image for efficiency
  char prefix [256]; 

}; 

int parse_options( param_t &options, int argc, const char **argv ); 


#endif
