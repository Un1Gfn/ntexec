#define WIN32_LEAN_AND_MEAN

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <winsock2.h>
#include <synchapi.h>
// #include <windows.h>
// #include <signal.h>
// #include <ws2tcpip.h>

#include "def.h"

static struct sockaddr_in to={};

static SOCKET s=-1;

static void load(){

  // https://docs.microsoft.com/en-us/cpp/c-runtime-library/argc-argv-wargv?view=vs-2019
  if(__argc!=2){
    printf("%s <DESTINATION_IP>\n",__argv[0]);
    exit(0);
  }

  WSADATA wsaData={};
  assert(0==WSAStartup(MAKEWORD(2,2),&wsaData));
  assert(2==LOBYTE(wsaData.wVersion)&&2==HIBYTE(wsaData.wVersion));

  // https://docs.microsoft.com/en-us/windows/win32/api/wsipv6ok/nf-wsipv6ok-inet_addr#code-try-1
  to=(struct sockaddr_in){
    .sin_family=AF_INET,
    .sin_port=htons(PORT),
    .sin_addr.s_addr=inet_addr(__argv[1]),
    // .sin_addr.s_addr=htonl(INADDR_LOOPBACK)
  };
  assert(to.sin_addr.s_addr!=INADDR_NONE);
  assert(to.sin_addr.s_addr!=INADDR_ANY);

  s=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
  assert(s!=INVALID_SOCKET);

  // maximum packet size of the underlying subnets
  unsigned int max_packet_size=0;
  getsockopt(s,SOL_SOCKET,SO_MAX_MSG_SIZE,(char*)(&max_packet_size),&((int){sizeof(unsigned int)}));
  assert(max_packet_size>=STRIDE);

  int sndbuf=0;

  assert(0==getsockopt(s,SOL_SOCKET,SO_SNDBUF,(char*)(&sndbuf),&((int){sizeof(int)})));
  printf("send buffer space %d bytes\n",sndbuf);

  assert(0==setsockopt(s,SOL_SOCKET,SO_SNDBUF,(const char*)(&((int){SND_BUF_SZ})),sizeof(int)));
  assert(0==getsockopt(s,SOL_SOCKET,SO_SNDBUF,(char*)(&sndbuf),&((int){sizeof(int)})));
  printf("send buffer space %d bytes\n",sndbuf);

}

int __cdecl main(){

  load();
 
  // char buf[SZ]={};
  // srand(time(NULL));
  // sprintf(buf,"%02d",rand()%100);
  // printf("send \"%s\" ...\n",buf);
  // assert(strlen(buf)==(size_t)sendto(s,buf,strlen(buf),0,(const struct sockaddr*)(&to),sizeof(to)));
  // printf("sent\n");

  unsigned char buf[COUNT][STRIDE];
  SecureZeroMemory(buf,sizeof(char)*COUNT*STRIDE);

  for(int i=0;i<COUNT*STRIDE;++i)
    *((unsigned char*)buf+i)=rand()%256;

  printf("send %d packets ...\n",COUNT);
  for(int i=0;i<COUNT;++i){
#define SENDTO sendto(s,(const char*)(buf[i]),STRIDE,0,(const struct sockaddr*)(&to),sizeof(to))
#if defined THROTTLE
    Sleep(INTERVAL);
    SENDTO;
#elif defined FAST
    SENDTO;
#else
    assert(STRIDE==SENDTO);
#endif
  }
  printf("sent\n");

  assert(0==closesocket(s));
  s=-1;
  assert(0==WSACleanup());
  // WSAGetLastError();

}
