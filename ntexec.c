#if!( defined(__linux__) && defined(__gnu_linux__) )
#error "Compile this file for Linux target!"
#endif

#include <assert.h>
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
#define URL_NO_HYPHEN(SRC) { url=SRC; assert( url && strlen(url)>=2 && url[0]!='-' && url[1]!='-' ); }

//            250~255   200~249     100~199     10~99     0~9
#define SEG "(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])"

#define S(x) (pmatch[x].rm_so)
#define E(x) (pmatch[x].rm_eo)

char conf_path[PATH_MAX]={};

static void help_exit(const char *const cmd){
  printf("\n");
  printf("[N] ""Open url remotely on a specific server ip\n");
  printf("  %s --<serveraddr_p> <url>\n",cmd);
  printf("  %s <url> --<serveraddr_p>\n",cmd);
  printf("\n");
  printf("[P] ""Open url remotely on the previous server ip\n");
  printf("  %s <url>\n",cmd);
  printf("\n");
  exit(0);
}

static void conf_genpath(){
  assert(
    conf_path==strcpy(conf_path,getenv("HOME")) && // /home/exampleuser
    conf_path==strcat(conf_path,"/"CONF)           // /home/exampleuser/CONF
  );
}

static void exit_if_ip_invalid(const char *const ip,const char *const fmt){
  regex_t reg={};
  assert(0==regcomp(&reg, "^(" SEG "\\.){3}" SEG "$" ,REG_EXTENDED));
  // regex(3)
  // The offsets of the subexpression starting at the ith open parenthesis are stored in pmatch[i]
  // The entire regular expression's match addresses are stored in pmatch[0]
  // Only Care about the entire, setting nmatch to 1
  regmatch_t pmatch[1]={{}};
  const int r=regexec(&reg,ip,1,pmatch,0);
  regfree(&reg);
  if(!(
    r==0 && S(0)==0 && (size_t)E(0)==strlen(ip) &&
    1==inet_aton(ip,&((struct in_addr){}))
  )){
    printf(fmt,conf_path);
    exit(-1);
  }
}

static void dump_close(char *const dest,FILE *const src,const int destspace){
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
static void conf_load(char *const dest,const int destlen){
  FILE *const f=fopen(conf_path,"r");
  if(!f){
    printf( "\n%s not found\n" "use [N] instead of [P]\n\n" , conf_path );
    exit(-1);
  }
  dump_close(dest,f,destlen);
  exit_if_ip_invalid(dest,"%s does not hold a valid IPv4 address\nfail to load\n");
}

// Mutually exclusive w/ conf_load()
static void conf_update(const char *const src){
  FILE *f=fopen(conf_path,"r");
  if(f){
    char t[INET_ADDRSTRLEN]={};
    dump_close(t,f,INET_ADDRSTRLEN);
    exit_if_ip_invalid(t,"%s already holds something but it is not a valid IPv4 address\nI would not dare modify it\n");
  }
  assert(NULL!=(f=fopen(conf_path,"w")));
  fprintf(f,"%s",src);
  assert(0==fclose(f));
}

static void ntexec(const char *const server,const char *const url){

  int sockfd=-1;

  // Test with loopback
  // const struct sockaddr_in serveraddr={
  //   .sin_family=AF_INET,
  //   .sin_port=htons(DEFAULT_PORT),
  //   .sin_addr={
  //     .s_addr=htonl(INADDR_LOOPBACK)
  //   }
  // };

  struct sockaddr_in serveraddr={
    .sin_family=AF_INET,
    .sin_port=htons(DEFAULT_PORT),
  };
  assert(1==inet_aton(server,&(serveraddr.sin_addr)));

  // Create socket
  assert(3<=(sockfd=socket(AF_INET,SOCK_DGRAM,0)));

  // Send
  assert(SZ>=strlen(url));
  printf("sending \"%s\" to %s ...\n",url,server);
  const ssize_t n=sendto(sockfd,url,strlen(url),MSG_CONFIRM,(const struct sockaddr*)(&serveraddr),SISZ);
  assert((long long)n==(long long)strlen(url));
  printf("sent\n");

  // Receive acknowledgement
  // char buf[SZ]={};
  // printf("trying to receive acknowledgement ...\n");
  // socklen_t addrlen=SISZ;
  // struct sockaddr_in backup={};
  // memcpy(&backup,&serveraddr,SISZ);
  // assert(1<=recvfrom(sockfd,buf,SZ,MSG_WAITALL,(struct sockaddr*)(&serveraddr),&addrlen));
  // assert(addrlen==SISZ);
  // assert(0==memcmp(&backup,&serveraddr,SISZ));
  // printf("received acknowledgement \"%s\"\n", buf);

  // Close socket
  close(sockfd);

}

int main(const int argc,const char *const *const argv){

  const char *url=NULL;

  bool serveraddr_is_new=false;             //         true               false
  char serveraddr_arr[INET_ADDRSTRLEN]={};  //          {}            from conf_path
  const char *serveraddr_p=NULL;            //  (argv[1]/argv[2])+2   serveraddr_arr

  // Locate conf
  conf_genpath();

  // Parse args (read conf)
  switch(argc-1){
  case 1:
    serveraddr_is_new=false;
    URL_NO_HYPHEN(argv[1]);
    conf_load(serveraddr_arr,INET_ADDRSTRLEN);
    serveraddr_p=serveraddr_arr;
    break;
  case 2:
    serveraddr_is_new=true;
    if(0==strncmp(argv[1],"--",2)){
      serveraddr_p=argv[1]+2;
      URL_NO_HYPHEN(argv[2]);
      exit_if_ip_invalid(serveraddr_p,"%.0sip address arg is invalid or contains extra whitespace\n");
    }else if(0==strncmp(argv[2],"--",2)){
      serveraddr_p=argv[2]+2;
      URL_NO_HYPHEN(argv[1]);
      exit_if_ip_invalid(serveraddr_p,"%.0sip address arg is invalid or contains extra whitespace\n");
    }else{
      help_exit(argv[0]);
    }
    break;
  default:
    help_exit(argv[0]);
  }

  // Ntexec
  ntexec(serveraddr_p,url);

  // Write conf
  serveraddr_is_new?conf_update(serveraddr_p):0;

  return 0;

}