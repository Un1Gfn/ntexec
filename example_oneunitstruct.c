// make example_oneunitstruct.out

#include <stdio.h>

typedef struct { int x; } X;
typedef struct { int y; } Y;
typedef struct { X x; Y y; } C;

C transpose(const C c){
  return (C){
    .x=(X){.x=c.y.y},
    .y=(Y){.y=c.x.x}
  };
}

void printc(const C c){
  printf("(%d,%d)\n",c.x.x,c.y.y);
}

int main(){
  const C c={{3},{4}};
  printc(c);
  printc(transpose(c));
}
