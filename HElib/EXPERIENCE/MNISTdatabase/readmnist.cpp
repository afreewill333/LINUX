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


void endianSwap(unsigned int &x) {
  x = (x>>24)|((x<<8)&0x00FF0000)|((x>>8)&0x0000FF00)|(x<<24);
}

int main() { 
  FILE *fimage = fopen("train-images.idx3-ubyte", "rb"); assert(fimage);
  FILE *flabel = fopen("train-labels.idx1-ubyte", "rb"); assert(flabel);  
  
  unsigned int magic, num, row, col;
  fread(&magic, 4, 1, fimage); assert(magic == 0x03080000);  
  fread(&magic, 4, 1, flabel); assert(magic == 0x01080000);
  

  fread(&num, 4, 1, flabel); // dust
  fread(&num, 4, 1, fimage); endianSwap(num);
  fread(&row, 4, 1, fimage); endianSwap(row);
  fread(&col, 4, 1, fimage); endianSwap(col);

  printf("num %d\n", num);
  printf("col %d\n", col);
  printf("row %d\n", row);

  for(int k=0;k<10;++k) { //num) {
    printf("---\n");
    vector<unsigned int> img(col*row);
    for(int i=0;i<col;++i)
	  {
		for(int j=0;j<row;++j)
		  {
		fread(&img[i*row+j], 1, 1, fimage);
		cout<<img[i*row+j];
		  }
		  cout<<endl;
	  }
    unsigned char label = 0;
    fread(&label, 1, 1, flabel);

    for(int i=0;i<col;++i) {
      for(int j=0;j<row;++j)
        printf("%2x", img[i*row+j]);
      printf("\n");
    }
    printf("  label = %x\n", label);
  }
}
