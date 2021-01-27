#include <assert.h>
#include <regex.h>
#include <stdio.h>

#define nmatch 3+1

//      OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
//      |                              |
#define S(x) (pmatch[x].rm_so) //      |
                               #define E(x) (pmatch[x].rm_eo)

//                        S(0)                               E(0)
//                        --------------------------------------
//                         S(1)   E(1)   S(2) E(2) S(3) E(3)
//                         -----------    -------- -------
const char *const tofind="(http|https)://(www\\.|)([a-z]+)\\.com";

const char *const string="http://github.com/982782394https://www.github.com/";

regex_t reg={};

regmatch_t pmatch[nmatch]={{}};

int main(){

  printf("\n");

  printf("%s\n",string);
  printf("%s\n",tofind);
  printf("\n");

  // Compile
  assert(0==regcomp(&reg,tofind,REG_EXTENDED));

  // Loop
  // for(const char *cursor=string; 0==(r=regexec(&reg,cursor,nmatch,pmatch,0)); cursor+=E(0)){
  const char *cursor=string;
  for(;;){
    const int r=regexec(&reg,cursor,nmatch,pmatch,0);
    if(r!=0){
      assert(r==REG_NOMATCH);
      break;
    }
    printf("%-25.*s ",E(0)-S(0),cursor+S(0));
    printf("%-25.*s ",E(3)-S(3),cursor+S(3));
    printf("\n");
    cursor+=E(0);
  }
  printf("\n");

  regfree(&reg);

  return 0;

}