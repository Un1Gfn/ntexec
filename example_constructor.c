#include <stdio.h>
#include <stdlib.h>

typedef unsigned char uchar;

uchar *buf=NULL;

__attribute__((constructor)) static void c(){
  buf=calloc(1024,sizeof(uchar));
  puts("(init)");
}
__attribute__((destructor)) static void d(){
  free(buf);
  buf=NULL;
  puts("(end)");
}

int main(){
  return EXIT_SUCCESS;
}
