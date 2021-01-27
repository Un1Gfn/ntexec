#if!( defined(__linux__) && defined(__gnu_linux__) )
#error "Compile this file for Linux target!"
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h> // strnlen
#include <unistd.h> // close()

#include <arpa/inet.h> // inet_aton()

#include "./def.h"
#include "./sock.h"

#define SISZ (sizeof(struct sockaddr_in))

static int sockfd=-1;
static struct sockaddr_in server={};

static void print_sin(const struct sockaddr_in *const sin0){
  const char *const s=inet_ntoa(sin0->sin_addr);
  assert(
    sin0->sin_family==AF_INET &&
    ntohs(sin0->sin_port)==DEFAULT_PORT &&
    s && s[0]
  );
  printf("%s:%u",s,DEFAULT_PORT);
  fflush(stdout);
}

void init0(const char *const ip){
  assert(3<=(sockfd=socket(AF_INET,SOCK_DGRAM,0)));
  // server=(struct sockaddr_in){
  //   .sin_family=AF_INET,
  //   .sin_port=htons(DEFAULT_PORT),
  //   .sin_addr={
  //     .s_addr=htonl(INADDR_LOOPBACK)
  //   }
  // };
  server=(struct sockaddr_in){
    .sin_family=AF_INET,
    .sin_port=htons(DEFAULT_PORT),
  };
  assert(1==inet_aton(ip,&(server.sin_addr)));
}

void send0(const char *const url){
  const size_t l=strnlen(url,SZ);
  assert(l<=SZ-1);
  printf("sending \"%s\" to ",url);
  print_sin(&server);
  printf(" ...");
  fflush(stdout);
  const ssize_t n=sendto(sockfd,url,l,MSG_CONFIRM,(const struct sockaddr*)(&server),SISZ);
  assert((long long)n==(long long)l);
  printf(" sent\n");
}

void recv0(){
  char buf[ACKSZ]={};
  socklen_t addrlen=SISZ;
  printf("waiting for acknowledgement ...\n");
  struct sockaddr_in sin2={};
  assert(4==recvfrom(sockfd,buf,ACKSZ,MSG_WAITALL,(struct sockaddr*)(&sin2),&addrlen));
  assert(addrlen==SISZ);
  assert(0==strncmp(buf,ACK,ACKSZ));
  printf("received acknowledgement from ");
  print_sin(&sin2);
  printf("\n");
}

void end0(){
  close(sockfd);
  sockfd=-1;
  bzero(&server,SISZ);
}
