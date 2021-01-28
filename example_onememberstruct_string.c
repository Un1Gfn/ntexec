#include <stdio.h>

typedef struct {
  char s[15+1];
} FixedString;

void f1(FixedString* s){

}

int main(int argc, char **argv){

  printf("%zu\n",sizeof(((FixedString*)NULL)->s));
  printf("%zu\n",sizeof(FixedString));

  FixedString s={};
  f1(&s);
  f1(argv[0]); // Warning

  return 0;
}