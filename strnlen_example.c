#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

void show(const bool b){
  printf(b?"true\n":"false\n");
}

bool check_ptr(const size_t max, const char *const ptr){
  const bool r=(strnlen(ptr,max+1)<=max);
  show(r);
  return r;
}

bool check_buf(const size_t max, char const buf[], const size_t buflen){
  assert(max<=buflen-1);
  const bool r=(buf[max]=='\0');
  show(r);
  return r;
}

int main(){

  printf("\n");

  char buf1[3+1]="ab";
  assert( check_buf(3,buf1,4));
  assert( check_buf(2,buf1,4));
  assert(!check_buf(1,buf1,4));
  assert(!check_buf(0,buf1,4));
  printf("\n");

  char buf2[3+1]="abc";
  assert( check_buf(3,buf2,4));
  assert(!check_buf(2,buf2,4));
  assert(!check_buf(1,buf2,4));
  assert(!check_buf(0,buf2,4));
  printf("\n");

  const char *const  ptr="abc";
  assert( check_ptr(3,ptr));
  assert(!check_ptr(2,ptr));
  assert(!check_ptr(1,ptr));
  assert(!check_ptr(0,ptr));
  printf("\n");

  return 0;

}