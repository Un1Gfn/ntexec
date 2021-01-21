#if ! ( defined(__linux__) && defined(__gnu_linux__) )
#error "Compile this file for Linux target!"
#endif

#include <assert.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "./def.h"

#define SISZ sizeof(struct sockaddr_in)

int sockfd=-1;

int main(const int argc,const char *const *const argv){

  // Help
  if(!(
    argc==3
    && strlen(argv[1])>=strlen("0.0.0.0")
    && strlen(argv[1])<=strlen("255.255.255.255")
    && strlen(argv[2])>=1
  )){
    printf("  %s <dest_ip> <url>\n",argv[0]);
    exit(0);
  }

  // To lo
  // const struct sockaddr_in serveraddr={
  //   .sin_family=AF_INET,
  //   .sin_port=htons(DEFAULT_PORT),
  //   .sin_addr={
  //     .s_addr=htonl(INADDR_LOOPBACK)
  //   }
  // };

  // To argv[1]
  struct sockaddr_in serveraddr={
    .sin_family=AF_INET,
    .sin_port=htons(DEFAULT_PORT),
  };
  assert(1==inet_aton(argv[1],&(serveraddr.sin_addr)));

  // Create socket
  assert(3<=(sockfd=socket(AF_INET,SOCK_DGRAM,0)));

  // Send
  printf("sending \"%s\" to %s ...\n",argv[2],argv[1]);
  const ssize_t n=sendto(sockfd,argv[2],strlen(argv[2]),MSG_CONFIRM,(const struct sockaddr*)(&serveraddr),SISZ);
  assert((long long)n==(long long)strlen(argv[2]));
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

  return 0;

}