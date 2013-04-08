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
  
  /* Write labeled image to file */ 
  void write( const char *fn ); 

  /* Label image and emit a reference to it */ 
  cv::Mat &labeled(); 

  /* Debugging display */ 
  void disp() const; 

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
      volume = 0; 
      centroid_x = centroid_y = 0; 
    }

    label_t *label; 
    int volume;
    int centroid_x, centroid_y; 
    int bbox [4];
  }; 
  
  /* Component accessors */ 
  component_t &operator[] (int); 
  const component_t &operator[] (int) const; 
  int size() const; 
    
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
    label_t &q = labels[i * cols + j]; 
    p = img.ptr<uchar>(i);
    for (j = 0; j < ncols; ++j)
    {
      labels[i * cols + j].pixel = p[j];
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
  
/* Component accessors */ 
ConnectedComponents::component_t &ConnectedComponents::operator[] (int i) 
{
  assert(i >= 0 && i < components_ct); 
  return components[i]; 
} // operator[]

const ConnectedComponents::component_t &ConnectedComponents::operator[] (int i) const
{
  assert(i >= 0 && i < components_ct); 
  return components[i]; 
} // operator[] const 

int ConnectedComponents::size() const
{
  return components_ct; 
} // site()

void ConnectedComponents::disp() const 
{
  int i,j; 
  // print
  for (i = 0; i < rows; i++) 
  {
    for (j = 0; j < cols; j++) 
    { 
      //printf("(%-1d,%-3d):", labels[i * cols +j].x, labels[i * cols + j].y); 
      if (labels[i * cols +j].label != UNASSIGNED)
        printf("%2d  ", labels[i * cols + j].label);
      else
        printf(" -  "); 
    }
    printf("\n"); 
  }

  for (i = 0; i < components_ct; i++) 
    printf("%d (%d, %d) vol=%d bbox=[%d, %d, %d, %d]\n", components[i].label->label, 
        components[i].centroid_x, components[i].centroid_y, components[i].volume,
        components[i].bbox[0], components[i].bbox[1], 
        components[i].bbox[2], components[i].bbox[3] ); 
} // disp() 


cv::Mat& ConnectedComponents::labeled() 
{
  int nrows = rows;
  int ncols = cols;

  if (img.isContinuous())
  {
    ncols *= nrows;
    nrows = 1;
  }
  int i, j, ct, label = 0; 

  uchar* p;
  for (i = 0; i < nrows; ++i)
  {
    label_t &q = labels[i * cols + j]; 
    p = img.ptr<uchar>(i);
    for (j = 0; j < ncols; ++j)
    {
      p[j] = (labels[i * cols + j].pixel * 10) % 255; /* FIXME */ 
    }
  }
  return img; 
} // labeled() 

void ConnectedComponents::write( const char *fn ) 
{
  cv::imwrite( fn, labeled() ); 
} // write()


void ConnectedComponents::ccomp() 
{
  int i, j, ct, label = 0; 
  label_t *neighbors [8]; 
  
  // first pass label 
  for (i = 0; i < rows; i++) 
  {
    for (j = 0; j < cols; j++) 
    {
      label_t &q = labels[i * cols + j]; 
      if (q.pixel > 0)
      {
        ct = getneighbors(neighbors, i, j);
        switch(ct) 
        {
          case 0: 
            q.label = label++;                // root
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

  // second pass relabel and calculate blob features
  for (i = 0; i < rows; i++) 
  {
    for (j = 0; j < cols; j++) 
    {
      label_t &q = labels[i * cols + j]; 
      if (q.pixel > 0)
      {
        label_t &p = _find( q );
        if (!p.component) {
          assert(components_ct < MAXCOMPS); 
          p.component = &components[components_ct++]; 
          p.component->bbox[0] = cols-1; 
          p.component->bbox[1] = 0; 
          p.component->bbox[2] = rows-1; 
          p.component->bbox[3] = 0; 
          p.component->label = &p;
        }
        component_t &c = *p.component;
        c.volume ++;
        c.bbox[0] = min(c.bbox[0], j); 
        c.bbox[1] = max(c.bbox[1], j); 
        c.bbox[2] = min(c.bbox[2], i); 
        c.bbox[3] = max(c.bbox[3], i);
        c.centroid_x += i; 
        c.centroid_y += j; 
        q.label = p.label;
        q.pixel = p.label % 255; /* FIXME */ 
      }
    }
  }

  // centroid calculation
  for (i = 0; i < components_ct; i++) {
    components[i].centroid_x /= components[i].volume; 
    components[i].centroid_y /= components[i].volume; 
  }
  
} // ccomp() 



void ConnectedComponents::_union (label_t &a, label_t &b)
{

  /* TODO This be done in O(1) if we maintain the rank of each node in the tree. 
   * Here we must traverse to the root for each union operation. */ 

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

  ConnectedComponents blobs( thresh ); 
  for (int i = 0; i < blobs.size(); i++) {
    ConnectedComponents::component_t &blob = blobs[i]; 
    printf("%d (%d, %d) vol=%d bbox=[%d, %d, %d, %d]\n", blob.label->label, 
        blob.centroid_x, blob.centroid_y, blob.volume,
        blob.bbox[0], blob.bbox[1], 
        blob.bbox[2], blob.bbox[3] ); 
  }
  blobs.write( "test.jpg" ); 


  //connectedComponents.disp();
  //connectedComponents.write( "test.jpg" );
   
  //cv::namedWindow( "Delta", CV_WINDOW_AUTOSIZE );
  //cv::imshow( "Delta", connectedComponents.labeled() );
  //cv::waitKey(0); 

  return 0; 

}
