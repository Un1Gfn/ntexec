// env LDLIBS=-ldl make example_dlopen.out

#include <assert.h>
#include <dlfcn.h>
#include <stddef.h>

int main(){

  // /usr/lib/libc.so.6 -> libc-*.so
  void *handle=dlopen("/usr/lib/libc.so.6",RTLD_LAZY|RTLD_LOCAL);
  assert(handle);

  assert(!dlerror());
  void *puts=dlsym(handle,"puts");
  assert(!dlerror());
  void *printf=dlsym(handle,"printf");
  assert(!dlerror());

  ((void(*)(const char *))puts)("lorem ipsum");
  ((int(*)(const char *__restrict __format, ...))printf)("%s %s %s\n","dolor","sit","amet");

  assert(0==dlclose(handle));
  handle=NULL;

  return 0;

}
