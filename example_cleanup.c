#include <stdlib.h>

typedef unsigned char uchar;

// Leak
void f1(){
  uchar *buf=calloc(1024,sizeof(uchar));
  ++*buf;
}

// No leak
void f2(){
  uchar *buf=calloc(1024,sizeof(uchar));
  free(buf);
  buf=NULL;
}

void cleanup_function(uchar *const *p){
  free(*p);
}

// No leak
void f3(){
  uchar *const buf __attribute__((cleanup(cleanup_function))) = calloc(1024,sizeof(uchar));
}

int main(){
  f1();
  f2();
  f3();
  return EXIT_SUCCESS;
}