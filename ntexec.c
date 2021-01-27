#if!( defined(__linux__) && defined(__gnu_linux__) )
#error "Compile this file for Linux target!"
#endif

#include <assert.h>
#include <ctype.h>
#include <limits.h> // PATH_MAX
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "./def.h"

#define SISZ (sizeof(struct sockaddr_in))
#define URL_NO_HYPHEN(SRC) { url=SRC; assert( url && url[0]!='-' && ( url[0]=='\0' || url[1]!='-' ) ); }

//            250~255   200~249     100~199     10~99     0~9
#define SEG "(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])"

#define S(x) (pmatch[x].rm_so)
#define E(x) (pmatch[x].rm_eo)

static void help_exit(const char *const cmd){
  printf("\n");
  printf("[N] ""Open url remotely on a specific server ip\n");
  printf("  %s --<serverip> <url>\n",cmd);
  printf("  %s <url> --<serverip>\n",cmd);
  printf("\n");
  printf("[P] ""Open url remotely on the previous server ip\n");
  printf("  %s <url>\n",cmd);
  printf("\n");
  printf("    ""Pass <url> as a single hyphen (-) to read urls from stdin\n");
  printf("\n");
  exit(0);
}

static void conf_genpath(char *const conf_path){
  assert(
    conf_path==strcpy(conf_path,getenv("HOME")) && // /home/exampleuser
    conf_path==strcat(conf_path,"/"CONF)           // /home/exampleuser/CONF
  );
}

static void exit_if_ip_invalid(const char *const ip, const char *const fmt, const char *const conf_path){
  assert( ip && ip[0]!='\0' );
  regex_t reg={};
  assert(0==regcomp(&reg, "^(" SEG "\\.){3}" SEG "$" ,REG_EXTENDED));
  // regex(3)
  // The offsets of the subexpression starting at the ith open parenthesis are stored in pmatch[i]
  // The entire regular expression's match addresses are stored in pmatch[0]
  // Only Care about the entire, setting nmatch to 1
  regmatch_t pmatch[1]={{}};
  const int r=regexec(&reg,ip,1,pmatch,0);
  regfree(&reg);
  const size_t l=strnlen(ip,INET_ADDRSTRLEN);
  if(!(
    r==0 && S(0)==0 && (size_t)E(0)==l && l<=(INET_ADDRSTRLEN-1) &&
    1==inet_aton(ip,&((struct in_addr){}))
  )){
    conf_path?printf(fmt,conf_path):printf(fmt);
    exit(-1);
  }
}

static void dump_close(char *const dest, FILE *const src, const int destspace){
  assert(
    0==fseek(src,0,SEEK_END) &&
    0==feof(src) &&
    0==ferror(src)
  );
  const long l=ftell(src);
  assert(l<=destspace-1);
  rewind(src);
  assert((long long)l==(long long)fread(dest,1,l,src));
}

// Mutually exclusive w/ conf_update()
static void conf_load(char *const dest, const int destlen, const char *const conf_path){
  FILE *const f=fopen(conf_path,"r");
  if(!f){
    printf( "\n%s not found\n" "use [N] instead of [P]\n\n" , conf_path );
    exit(-1);
  }
  dump_close(dest,f,destlen);
  exit_if_ip_invalid(dest,"%s does not hold a valid IPv4 address\nfail to load\n",conf_path);
}

// Mutually exclusive w/ conf_load()
static void conf_update(const char *const conf_path, const char *const src){
  FILE *f=fopen(conf_path,"r");
  if(f){
    char t[INET_ADDRSTRLEN]={};
    dump_close(t,f,INET_ADDRSTRLEN);
    exit_if_ip_invalid(t,"%s already holds something but it is not a valid IPv4 address\nI would not dare modify it\n",conf_path);
  }
  assert(NULL!=(f=fopen(conf_path,"w")));
  fprintf(f,"%s",src);
  assert(0==fclose(f));
}

static int init0(){
  int ret=-1;
  // Create socket
  assert(3<=(ret=socket(AF_INET,SOCK_DGRAM,0)));
  return ret;
}

static void send0(const char *const server, const int sockfd, const char *const url){
  // Test with loopback
  // const struct sockaddr_in serveraddr={
  //   .sin_family=AF_INET,
  //   .sin_port=htons(DEFAULT_PORT),
  //   .sin_addr={
  //     .s_addr=htonl(INADDR_LOOPBACK)
  //   }
  // };
  // Load address
  struct sockaddr_in sin={
    .sin_family=AF_INET,
    .sin_port=htons(DEFAULT_PORT),
  };
  assert(1==inet_aton(server,&(sin.sin_addr)));
  // Send
  const size_t l=strnlen(url,SZ);
  assert(l<=SZ-1);
  printf("sending \"%s\" to %s ...",url,server);
  fflush(stdout);
  const ssize_t n=sendto(sockfd,url,l,MSG_CONFIRM,(const struct sockaddr*)(&sin),SISZ);
  assert((long long)n==(long long)l);
  printf(" sent\n");
}

/*static void recv0(const int sockfd){

  assert(false);

  // getpeername()

  // char buf[ACKSZ]={};
  // struct sockaddr_in sin={};

  // socklen_t addrlen=SISZ;
  // printf("waiting for acknowledgement ...\n");
  // assert(4==recvfrom(sockfd,buf,ACKSZ,MSG_WAITALL,(struct sockaddr*)(&sin),&addrlen));
  // assert(addrlen==SISZ);

  // const uint32_t port=ntohl(sin.sin_port);
  // const char *const s=inet_ntoa(sin.sin_addr);
  // assert(
  //   0==strncmp(buf,ACK,ACKSZ) &&
  //   sin.sin_family==AF_INET &&
  //   port==DEFAULT_PORT &&
  //   s && s[0]
  // );
  // printf("received acknowledgement from %s:%u\n",s,port);

}*/

// Return false if the line is empty/blank
// Return true if the line is copied and ready for sending
static bool read0(bool *lastone, char *const buf, const size_t buflen){

  // https://stackoverflow.com/a/1516177
  // (TNE)  isatty(fileno(stdin) xxx\n[EOF]
  // (PNE) !isatty(fileno(stdin) xxx\n[EOF]
  // (TFSE)   isatty(fileno(stdin) xxx[FLUSH][EOF] // One ^D flushes stdin - Two consecutive ^D's gives EOF
  // (PE)  !isatty(fileno(stdin) xxx[EOF]

  bzero(buf,buflen);
  char c='\0';
  #define GETCHAR() { c=getchar(); assert( (EOF==c||c>=1) && !ferror(stdin) ); }

  //     ' '  ' '  'a'  'b'  '\n'
  //  |
  //  c

  // (TNE) or (PNE)
  GETCHAR();
  if(feof(stdin)||c==EOF){
    assert(feof(stdin)&&c==EOF);
    *lastone=true;
    return false;
  }

  // Skip leading spaces and tabs
  while(isblank(c)){
    GETCHAR();
    if(feof(stdin)||c==EOF){ // (TFSE) or (PE)
      assert( feof(stdin) && c==EOF );
      isatty(fileno(stdin))?printf("\n"):0; // (TFSE)
      *lastone=true;
      return false;
    }
  }

  //     ' '  ' '  'a'  'b'  '\n'
  //                |
  //                c

  bool ret=false;
  size_t i=0;

  for(;;){
    assert(i<=buflen-1);
    if(feof(stdin)||c==EOF){ // (TFSE) or (PE)
      assert( feof(stdin) && c==EOF );
      isatty(fileno(stdin))?printf("\n"):0; // (TFSE)
      *lastone=true;
      return ret;
    }
    if(c=='\n'){
      buf[i]='\0';
      return ret;
    }
    buf[i]=c;
    ret=true;
    GETCHAR();
    ++i;
  }

  //     ' '  ' '  'a'  'b'  '\n'
  //                          |
  //                          c

  // Must not reach here
  assert(false);
  return false;

}

static void ntexec(const char *const srv, const char *const argv_pattern){
  assert(argv_pattern[0]!='\0');
  const int sockfd=init0();
  if(argv_pattern[0]=='-'){
    assert(argv_pattern[1]=='\0');
    char buf[SZ]={};
    bool lastone=false;
    for(;;){
      if(lastone)
        break;
      if(!read0(&lastone,buf,SZ))
        continue;
      send0(srv,sockfd,buf);
      // recv0(sockfd);
    }
  }else{
    send0(srv,sockfd,argv_pattern);
  }
  close(sockfd);
}

int main(const int argc, const char *const *const argv){

  // Locate conf
  char conf_path[PATH_MAX]={};
  conf_genpath(conf_path);

  // Parse args (read conf)
  switch(argc-1){
  case 1:{
    char srvip_arr[INET_ADDRSTRLEN]={};
    conf_load(srvip_arr,INET_ADDRSTRLEN,conf_path);
    ntexec(srvip_arr,argv[1]);
    break;
  }break;
  case 2:
    if(0==strncmp(argv[1],"--",2)){
      // With the trailing NULL, there is no need to hide string arg with "%.0s" hack 
      exit_if_ip_invalid(argv[1]+2,"ip address arg is invalid or contains extra whitespace\n",NULL);
      conf_update(conf_path,argv[1+2]);
      ntexec(argv[1]+2,argv[2]);
    }else if(0==strncmp(argv[2],"--",2)){
      exit_if_ip_invalid(argv[2]+2,"ip address arg is invalid or contains extra whitespace\n",NULL);
      conf_update(conf_path,argv[2+2]);
      ntexec(argv[2]+2,argv[1]);
    }else{
      help_exit(argv[0]);
    }
    break;
  default:
    help_exit(argv[0]);
  }

  return 0;

}
