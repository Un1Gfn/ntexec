// https://gcc.gnu.org/onlinedocs/gcc-10.2.0/gcc/Labels-as-Values.html#Labels-as-Values
// avr-gcc example_labels-as-values.c -Wall -Wextra -nostdlib

#ifndef __AVR__
#include <stdio.h>
#else
static void printg(const char *const s){

}
#endif

static void f(int i){

  if(0<=i&&i<=2)
    return;

  // static const void *(arr[3])={&&foo,&&bar,&&baz};
  // goto *(arr[i]);

  static const int array[3]={
    &&foo-&&foo,
    &&bar-&&foo,
    &&baz-&&foo
  };
  goto *(&&foo+array[i]);

  #ifndef __AVR__
  foo:printf("foo\n");return;
  bar:printf("bar\n");return;
  baz:printf("baz\n");return;
  #else
  foo:printg("foo\n");return;
  bar:printg("bar\n");return;
  baz:printg("baz\n");return;
  #endif
}

int main(){
  f(2);
  f(1);
  f(0);
  return 0;
}
