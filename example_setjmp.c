#include <assert.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define eprintf(...) fprintf(stderr,__VA_ARGS__)

static jmp_buf env={};
static const char *const eh_msg[]={NULL,"err1","err2"};


static void subproc1(){eprintf("subproc1() ");}

static int subproc2_behav=0;
static void subproc2(){eprintf("subproc2() ");
  if(subproc2_behav){
    eprintf("fail ");
    longjmp(env,subproc2_behav);
  }
}

static void subproc3(){eprintf("subproc3() ");}

/*static void proc(){
  bzero(env,sizeof(jmp_buf));
  const int exception_type=setjmp(env);
  if(exception_type==0){                // try {
    subproc1();
    subproc2();
    subproc3();
    eprintf("succeed\n");
  }else switch(exception_type){         // }
    case 1:                // catch(1) {
      eprintf("err1\n");
      break;               // }
    case 2:                // catch(2) {
      eprintf("err2\n");
      break;               // }
    default:               // catch (...) {
      eprintf("unkerr\n");
      break;               // }
  }
  bzero(env,sizeof(jmp_buf));
}*/

static void proc(){

  // (A)
  // bzero(env,sizeof(jmp_buf));
  // assert( 0==*((unsigned char*)env) && 0==memcmp(env,((unsigned char*)env)+1,sizeof(jmp_buf)-1) );

  // (B)
  jmp_buf env_backup={};
  memcpy(env_backup,env,sizeof(jmp_buf));

  const int exception_type=setjmp(env);
  if(exception_type==0){  // try {
    subproc1();
    subproc2();
    subproc3();
    eprintf("succeed\n");
  }else{                  // } catch (...)
    assert( 0<exception_type && (size_t)exception_type<(sizeof(eh_msg)/sizeof(const char *const)) );
    eprintf("%s\n",eh_msg[exception_type]);
  }                       // }

  // (A)
  // bzero(env,sizeof(jmp_buf));

  // (B)
  memcpy(env,env_backup,sizeof(jmp_buf));

}

int main(){

  subproc2_behav=0;proc();
  subproc2_behav=1;proc();
  subproc2_behav=2;proc();
  subproc2_behav=99;proc();

  return EXIT_SUCCESS;

}
