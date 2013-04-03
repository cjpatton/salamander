#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "highgui.h"
#include <iostream>
#include <cstdio>

using namespace std; 

#define min(x,y) ( x < y ? x : y )
#define max(x,y) ( x > y ? x : y )
#define UNASSIGNED -1


struct label_t {

  uchar pixel; 
  int   label;
  label_t *parent; 

}; 



int getneighbors(label_t* mat, const int rows, const int cols, label_t* neighbors [], int i, int j) 
/* Return a list of foreground neighboring pixels that have been
 * assigned labels. */ 
{
  int ct = 0;

  printf("(%d, %d) ", i, j); 
  for (int x = max(0, i-1); x <= min(i+1, rows-1); x++)
  {
    for (int y = max(0, j-1); y <= min(j+1, cols-1); y++)
    {
      if (mat[x * cols + y].label != UNASSIGNED) {
        cout << mat[x * cols + y].label << ' '; 
        neighbors[ct++] = &mat[x * cols + y];
      }
    }
  }
  cout << endl;


//  if (i > 0) 
//  {
//    if (j > 0 && mat[(i-1) * cols + (j-1)].label != UNASSIGNED) 
//        neighbors[ct++] = &mat[(i-1) * cols + (j-1)];
//
//    if (mat[(i-1) * cols + j].label != UNASSIGNED) 
//      neighbors[ct++] = &mat[(i-1) * cols + j];
//  }
//
//  if (j > 0 && mat[i * cols + (j-1)].label != UNASSIGNED) 
//        neighbors[ct++] = &mat[i * cols + (j-1)];
    


  return ct; 
}


void Union (label_t &a, label_t &b) {
  label_t *A = a.parent, *B = b.parent, *prev; 
  int rankA = 0, rankB = 0; 

  prev = &a; 
  while (A) {
    prev = A; 
    A = A->parent; 
    rankA ++; 
  }
  A = prev;
  
  prev = &b; 
  while (B) {
    prev = B; 
    B = B->parent; 
    rankB ++; 
  }
  B = prev;
  
  if (A != B)
  {
    if (rankA < rankB) 
      A->parent = B; 
  
    else
      B->parent = A; 
  }

}

label_t &Find (label_t &a) 
{
  label_t *A = a.parent, *prev; 
  prev = &a; 
  while (A) {
    prev = A;
    A = A->parent; 
  }
  A = prev; 
  return *A;  
}


void ccomp(cv::Mat& img)
{
  CV_Assert(img.depth() == CV_8U);  // accept only uchar images
  CV_Assert(img.channels() == 1);   // just one channel

  label_t *labels = new label_t [img.cols * img.rows];
  label_t *neighbors [8]; 

  int nrows = img.rows;
  int ncols = img.cols;

  if (img.isContinuous())
  {
    ncols *= nrows;
    nrows = 1;
  }

  // populate label matrix
  int i, j, ct, label = 0; 
  uchar* p;
  for (i = 0; i < nrows; ++i)
  {
    label_t &q = labels[i * img.cols + j]; 
    p = img.ptr<uchar>(i);
    for (j = 0; j < ncols; ++j)
    {
      labels[i * img.cols + j].pixel = p[j];
      labels[i * img.cols + j].label = UNASSIGNED; 
      labels[i * img.cols + j].parent = NULL; 
    }
  }
  
  // first pass label 
  for (i = 0; i < img.rows; i++) 
  {
    for (j = 0; j < img.cols; j++) 
    {
      label_t &q = labels[i * img.cols + j]; 
      if (q.pixel > 0)
      {
        ct = getneighbors(labels, img.rows, img.cols, neighbors, i, j);
        switch(ct) 
        {
          case 0: 
            q.label = label++;                // root
            break;

          case 1: 
            q.label =  neighbors[0]->label;   // sibling
            q.parent = neighbors[0]->parent; 
            Union(q, *neighbors[0]); 
            break;

          default: 
            label_t *min = neighbors[0]; 
            for (int k = 1; k < ct; k++) 
            {
              if (neighbors[k]->label < min->label)
                min = neighbors[k]; 
            }
            q.label  = min->label;             // sibling of min label
            q.parent = min->parent; 

            for (int k = 0; k < ct; k++)       // neighbors are in the same group
              Union(q, *neighbors[k]); 

        }
      }
    }
  }

  // second pass relabel 
  for (i = 0; i < img.rows; i++) 
  {
    for (j = 0; j < img.cols; j++) 
    {
      label_t &q = labels[i * img.cols + j]; 
      if (1 || q.pixel > 0)
      {
        label_t &p = Find( q ); 
        q.label = p.label; 
      }
    }
  }

  // print
  for (i = 0; i < img.rows; i++) 
  {
    for (j = 0; j < img.cols; j++) 
    { 
      if (labels[i * img.cols +j].pixel > 0)
        printf("%2d  ", labels[i * img.cols + j].label);
      else
        printf(" -  "); 
    }
    printf("\n"); 
  }

  delete [] labels; 

}



int main(int argc, const char **argv) {

  if (argc != 2) {
    cerr << "usage: detect image.jpg\n";
    return 1; 
  }
  
  cv::Mat img, thresh; 

  img = cv::imread( argv[1], CV_LOAD_IMAGE_GRAYSCALE ); 
  cv::threshold(img, thresh, 100, 255, CV_THRESH_BINARY); /* Threshold value doesn't matter */ 
  ccomp(thresh); 
  cv::imwrite( "test.jpg", thresh ); 

  return 0; 

}
