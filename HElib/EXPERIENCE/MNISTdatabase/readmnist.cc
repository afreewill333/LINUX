// Read MNIST Database (handwritten digits)
//
// Usage: 
//   1. download 
//        train-images-idx3-ubyte.gz
//        train-labels-idx2-ubyte.gz
//      from
//        http://yann.lecun.com/exdb/mnist/
//      and extract them.
//
//    2. ./a
//

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <map>
#include <vector>
#include <cstring>
#include <functional>
#include <algorithm>
#include <cassert>

using namespace std;

#define ALL(c) c.begin(), c.end()
#define FOR(i,c) for(typeof(c.begin())i=c.begin();i!=c.end();++i)
#define REP(i,n) for(int i=0;i<n;++i)
#define fst first
#define snd second

void endianSwap(unsigned int &x) {
  x = (x>>24)|((x<<8)&0x00FF0000)|((x>>8)&0x0000FF00)|(x<<24);
}
typedef vector<unsigned int> Image;

int main() { 
  FILE *fimage = fopen("train-images.idx3-ubyte", "rb");
  FILE *flabel = fopen("train-labels.idx1-ubyte", "rb");
  assert(fimage);
  assert(flabel);
  
  unsigned int magic, num, row, col;
  fread(&magic, 4, 1, fimage); 
  assert(magic == 0x03080000);
  fread(&magic, 4, 1, flabel);
  assert(magic == 0x01080000);

  fread(&num, 4, 1, flabel); // dust
  fread(&num, 4, 1, fimage); endianSwap(num);
  fread(&row, 4, 1, fimage); endianSwap(row);
  fread(&col, 4, 1, fimage); endianSwap(col);

  printf("num %d\n", num);
  printf("col %d\n", col);
  printf("row %d\n", row);

  REP(k, 10) { //num) {
    printf("---\n");
    Image img(col*row);
    REP(i, col) REP(j, row) 
      fread(&img[i*row+j], 1, 1, fimage);
    unsigned char label = 0;
    fread(&label, 1, 1, flabel);

    REP(i, col) {
      REP(j, row) 
        printf("%02x", img[i*row+j]);
      printf("\n");
    }
    printf("  label = %x\n", label);
  }
}
