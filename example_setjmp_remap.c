/*

https://en.wikipedia.org/wiki/Setjmp.h#Exception_handling

make -B example_setjmp_remap.out &&
diff -u <(./example_setjmp_remap.out ) <(echo \
'calling first
entering first
calling second
entering second
second failed, exception type: 3; remapping to type 1
first failed, exception type: 1')

*/

#include <assert.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Use a file scoped static variable for the exception stack so we can access
 * it anywhere within this translation unit. */
static int exception_type=-1;
static jmp_buf env={};

static void second(){
  puts("entering second");
  exception_type=3;
  longjmp(env,exception_type); // (A) Throw exception
  puts("leaving second"); // not reached
}

static void first(){

  puts("entering first");
  jmp_buf env_backup={};
  memcpy(env_backup,env,sizeof(jmp_buf));

  const int e=setjmp(env);
  if(e==0){                    // try {
    puts("calling second");
    second();
    puts("second succeeded");
    memcpy(env, env_backup, sizeof(jmp_buf));
    puts("leaving first"); // not reached
  }else switch(e){             // } catch(...){
    case 3:
      puts("second failed, exception type: 3; remapping to type 1");
      exception_type=1;
      __attribute__((fallthrough));
    default:
      memcpy(env,env_backup,sizeof(jmp_buf));
      longjmp(env,exception_type); // (B) Pass over thrown exception
      assert(false);
      break;
  }                            // }

}

int main(void){
  // https://en.wikipedia.org/wiki/Volatile_(computer_programming)
  char *volatile mem_buffer=NULL;
  if(0==setjmp(env)){ // try {
    puts("calling first");
    first();
    // not reached
    printf("%s\n",strcpy(mem_buffer=malloc(300),"first succeeded"));
  }else{              // } catch(...) {
    printf("first failed, exception type: %d\n", exception_type);
  }                   // }
  free(mem_buffer);
  return EXIT_SUCCESS;
}
