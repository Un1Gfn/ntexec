/*
env LDLIBS="$(pkg-config --libs libcrypto)" make -B example_setjmp_coopmtk.out && ./example_setjmp_coopmtk.out | head -30
*/

#include <assert.h>
#include <stdbool.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h> // EXIT_FAILURE
#include <openssl/sha.h>

jmp_buf e_main={};
jmp_buf e_child={};
static_assert(sizeof(jmp_buf)==200);

#define longjmp1(ENV) longjmp(ENV,1)

bool is_empty(jmp_buf env){
  const unsigned char *const p=(const unsigned char *)env;
  return(
    p[0]==0 &&
    0==memcmp(p,p+1,sizeof(jmp_buf)-1)
  );
}

// #define STR(X) #X
#define printh(COLOR,MSG,ENV) { printf("\033[%dm%s ",COLOR,MSG/*STR(ENV)*/); printh0(ENV); printf("\033[0m\n"); fflush(stdout); }
void printh0(jmp_buf env){
  // printf("%zu ",sizeof(jmp_buf));
  unsigned char md[SHA_DIGEST_LENGTH]={};
  assert(md==SHA1((const unsigned char*)env,sizeof(jmp_buf),md));
  for(int i=0;i<SHA_DIGEST_LENGTH;++i)
    printf("%02X",md[i]);
}

// void count_changes(jmp_buf env){
//   static bool changed;
//   static jmp_buf env2;
//   // Not yet changed
//   if(is_empty(env))
//     return;
//   // First change
//   if(!changed){
//     memcpy(env2,env,sizeof(jmp_buf));
//     changed=true;
//     return;
//   }
//   // Further calls
//   assert(changed);
//   assert(0==memcmp(env,env2,sizeof(jmp_buf)));
// }

void child(){
  int r=0;
  for(;;){
    printf("Child loop begin\n");

    r=setjmp(e_child);
    // printh(32,"child (1)",e_child);
    if(!r)
      longjmp1(e_main); // yield - invalidates e_child in C99

    printf("Child loop end\n");

    r=setjmp(e_child);
    // printh(32,"child (2)",e_child);
    if(!r)
      longjmp1(e_main); // yield - invalidates e_child in C99
  }

  /* Don't return. Instead we should set a flag to indicate that main()
     should stop yielding to us and then longjmp(e_main, 1) */
}

void call_with_cushion(){
  #pragma GCC diagnostic ignored "-Wunused-variable"
  volatile char space[1000]; // Reserve enough space for main to run
  child();
}

int main(){

  int r=0;

  assert(is_empty(e_main));
  r=setjmp(e_main);
  // printh(34,"main(1)",e_main);
  if(!r){
    call_with_cushion(); // child never returns, yield
    assert(false);
  }

  for(;;){
    printf("Parent\n");
    r=setjmp(e_main);
    // printh(34,"main(2)",e_main);
    if(!r)
      longjmp1(e_child); // yield - note that this is undefined under C99
  }

  assert(false);
  return EXIT_FAILURE;

}
