#include <assert.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define eprintf(...) fprintf(stderr,__VA_ARGS__)

jmp_buf env={};

int errcode=0;

static void subproc1(){eprintf("subproc1() ");}
static void subproc2(){eprintf("subproc2() ");
  if(errcode){
    eprintf("fail ");
    longjmp(env,errcode);
  }
}
static void subproc3(){eprintf("subproc3() ");}

static void proc(){
  const int r=setjmp(env);
  switch(r){
  case 0:                  // try {
    subproc1();
    subproc2();
    subproc3();
    eprintf("succeed\n");
    break;                 // }
  case 1:                  // catch(1) {
    eprintf("err1\n");
    break;                 // }
  case 2:                  // catch(2) {
    eprintf("err2\n");
    break;                 // }
  default:                 // catch (...) {
    eprintf("unkerr\n");
    break;                 // }
  }
}

int main(){

  errcode=0;proc();
  errcode=1;proc();
  errcode=2;proc();
  errcode=99;proc();

  return EXIT_SUCCESS;

}
