#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "highgui.h"
#include <iostream>
#include <cstdio>

using namespace std; 



#define min(x,y) ( x < y ? x : y )
#define max(x,y) ( x > y ? x : y )
#define UNASSIGNED -1
#define MAXCOMPS 1024

class ConnectedComponents
{

public: 

  ConnectedComponents( const cv::Mat& ); 
  
  ~ConnectedComponents(); 
  
  void disp() const; 
  
  void write( const char *fn ); 

  cv::Mat &labeled(); 

  void calc_blobs();  

  struct component_t; 

  /* Disjoint-set data structure and cc matrix cell */ 
  struct label_t {
    
    label_t() {
      label = UNASSIGNED; 
      parent = NULL; 
      component = NULL; 
    }

    uchar pixel; 
    int   label;
    label_t *parent;
    component_t *component; 
  };

  struct component_t {

    component_t() {
      label = NULL; 
      x = y = 0; 
      volume = 0; 
      centroid_x = centroid_y = 0; 
    }

    label_t *label; 
    int x, y;  
    int volume;
    int centroid_x, centroid_y; 
    int bbox [4];
  }; 
    
private:

  /* Disjoint-set methods */
  void      _union (label_t &a, label_t &b);
  label_t & _find (label_t &a) const; 
    
  /* Connected component analysis */ 
  int getneighbors(label_t* neighbors [], int i, int j);
  void ccomp();
  
  label_t     *labels; 
  component_t *components; 
  int rows, cols, components_ct; 
  
  cv::Mat img; 
   
}; // class ConnectedComponents


ConnectedComponents::ConnectedComponents( const cv::Mat &anImage )
{
  CV_Assert(anImage.depth() == CV_8U);  // accept only uchar images
  CV_Assert(anImage.channels() == 1);   // just one channel

  img = anImage.clone();
  rows = img.rows; 
  cols = img.cols;
  components_ct = 0; 
  
  labels = new label_t [rows * cols]; 
  components = new component_t [MAXCOMPS]; 

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
    }
  }
  
  // perform connected component analysis
  ccomp(); 
  
} // constr

ConnectedComponents::~ConnectedComponents() 
{
  delete [] labels;
  delete [] components;
} // destr

void ConnectedComponents::disp() const 
{
  int i,j; 
  // print
  for (i = 0; i < img.rows; i++) 
  {
    for (j = 0; j < img.cols; j++) 
    { 
      //printf("(%-1d,%-3d):", labels[i * img.cols +j].x, labels[i * img.cols + j].y); 
      if (labels[i * img.cols +j].label != UNASSIGNED)
        printf("%2d  ", labels[i * img.cols + j].label);
      else
        printf(" -  "); 
    }
    printf("\n"); 
  }

  for (i = 0; i < components_ct; i++) 
    printf("%d (%d, %d) vol=%d\n", components[i].label->label, 
        components[i].x, components[i].y, components[i].volume); 
} // disp() 


cv::Mat& ConnectedComponents::labeled() 
{
  int nrows = img.rows;
  int ncols = img.cols;

  if (img.isContinuous())
  {
    ncols *= nrows;
    nrows = 1;
  }
  int i, j, ct, label = 0; 

  uchar* p;
  for (i = 0; i < nrows; ++i)
  {
    label_t &q = labels[i * img.cols + j]; 
    p = img.ptr<uchar>(i);
    for (j = 0; j < ncols; ++j)
    {
      p[j] = (labels[i * img.cols + j].pixel * 10) % 255; /* FIXME */ 
    }
  }
  return img; 
} // labeled() 

void ConnectedComponents::write( const char *fn ) 
{
  cv::imwrite( fn, labeled() ); 
} // write()

void ConnectedComponents::calc_blobs() 
{


}



void ConnectedComponents::ccomp() 
{
  int i, j, ct, label = 0; 
  label_t *neighbors [8]; 
  
  // first pass label 
  for (i = 0; i < img.rows; i++) 
  {
    for (j = 0; j < img.cols; j++) 
    {
      label_t &q = labels[i * img.cols + j]; 
      if (q.pixel > 0)
      {
        ct = getneighbors(neighbors, i, j);
        switch(ct) 
        {
          case 0: 
            q.label = label++;                // root
            break;

          case 1: 
            q.label =  neighbors[0]->label;   // sibling
            q.parent = neighbors[0]->parent; 
            _union(q, *neighbors[0]); 
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
              _union(q, *neighbors[k]); 

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
      if (q.pixel > 0)
      {
        label_t &p = _find( q );
        if (!p.component) {
          p.component = &components[components_ct++]; 
          p.component->bbox[0] = 0; 
          p.component->bbox[1] = img.cols-1; 
          p.component->bbox[2] = 0; 
          p.component->bbox[3] = img.rows-1; 
          p.component->x = i;
          p.component->y = j;
          p.component->label = &p;
        }
        p.component->volume ++;
        /* TODO bbox and centroid */ 
        q.label = p.label;
        q.pixel = p.label % 255; /* FIXME */ 
      }
    }
  }
  
} // ccomp() 



void ConnectedComponents::_union (label_t &a, label_t &b)
{
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
} // _union()

ConnectedComponents::label_t &ConnectedComponents::_find (label_t &a) const
{
  label_t *A = a.parent, *prev; 
  prev = &a; 
  while (A) {
    prev = A;
    A = A->parent; 
  }
  A = prev; 
  return *A;  
} // _find()

int ConnectedComponents::getneighbors(label_t* neighbors [], int i, int j) 
{
  int ct = 0;

  //printf("(%d, %d) ", i, j); 
  for (int x = max(0, i-1); x <= min(i+1, rows-1); x++)
  {
    for (int y = max(0, j-1); y <= min(j+1, cols-1); y++)
    {
      if (labels[x * cols + y].label != UNASSIGNED) {
        //cout << labels[x * cols + y].label << ' '; 
        neighbors[ct++] = &labels[x * cols + y];
      }
    }
  }
  //cout << endl;
  
  return ct; 
} // getneighbors()


int main(int argc, const char **argv) {

  if (argc != 2) {
    cerr << "usage: detect image.jpg\n";
    return 1; 
  }
  
  cv::Mat img, thresh; 

  img = cv::imread( argv[1], CV_LOAD_IMAGE_GRAYSCALE ); 
  cv::threshold(img, thresh, 100, 255, CV_THRESH_BINARY); /* Threshold value doesn't matter */ 

  ConnectedComponents connectedComponents( thresh ); 
  connectedComponents.disp();
  //connectedComponents.write( "test.jpg" );
   
  //cv::namedWindow( "Delta", CV_WINDOW_AUTOSIZE );
  //cv::imshow( "Delta", connectedComponents.labeled() );
  //cv::waitKey(0); 

  return 0; 

}
