// https://jweinst1.medium.com/how-to-use-udp-sockets-on-windows-29e7e60679fe

// https://stackoverflow.com/questions/142508/how-do-i-check-os-with-a-preprocessor-directive
// https://sourceforge.net/p/predef/wiki/OperatingSystems/
#ifndef _WIN64
#error "Compile this file for WIN64 target!"
#endif

// #ifndef WIN32_LEAN_AND_MEAN
// #define WIN32_LEAN_AND_MEAN
// #endif
// #include <windows.h>

#include <winsock2.h>
#include <ws2tcpip.h>
// #include <Iphlpapi.h> // IP Helper API

#include <shlobj.h> // ShellExecute()

#include <assert.h>
#include <stdio.h>

#include "./def.h"

// https://docs.microsoft.com/en-us/cpp/preprocessor/comment-c-cpp
// #pragma comment(lib, "Ws2_32.lib")

SOCKET sockfd=INVALID_SOCKET;

void openlnk(){
  ShellExecute(NULL,"open","http://www.google.com/",NULL,NULL,SW_SHOWNORMAL);
}

void initialize_winsock(){
  // Initialize Winsock
  WSADATA wsaData={};
  ZeroMemory(&wsaData,sizeof(WSADATA));
  assert(
    0==WSAStartup(MAKEWORD(2,2),&wsaData)
    // The version of the Windows Sockets specification that the Ws2_32.dll expects the caller to use
    && (2==(LOBYTE(wsaData.wVersion))) // major version number 
    && (2==(HIBYTE(wsaData.wVersion))) // minor version number 
    // The highest version of the Windows Sockets specification that the Ws2_32.dll can support
    && (2==(LOBYTE(wsaData.wHighVersion))) // major version number 
    && (2==(HIBYTE(wsaData.wHighVersion))) // minor version number 
  );
}

void create_socket(){
  assert(INVALID_SOCKET!=(sockfd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)));
  // WSAGetLastError()
}

void bind_socket(){
  struct sockaddr_in server={};
  ZeroMemory(&server,sizeof(struct sockaddr_in));
  server=(struct sockaddr_in){
    // Bind the socket to any address and the specified port.
    .sin_family=AF_INET,
    .sin_port=htons(DEFAULT_PORT),
    .sin_addr.s_addr=htonl(INADDR_ANY)
    // .sin_addr.s_addr = inet_addr("127.0.0.1")
  };
  assert(0==bind(sockfd,(SOCKADDR*)(&server),sizeof(struct sockaddr_in)));
}

void receive_data(){

  char buf[SZ+1]={};
  ZeroMemory(buf,SZ+1);

  struct sockaddr_in client={};
  ZeroMemory(&client,sizeof(struct sockaddr_in));

  // printf("Receiving datagrams on %s\n", "127.0.0.1");
  const int bytes_received=recvfrom(sockfd,buf,SZ,0/* no flags*/,(SOCKADDR*)(&client),&((int){sizeof(client)}));
  assert(
    bytes_received!=SOCKET_ERROR &&
    bytes_received>=1 &&
    bytes_received<=SZ
  );
  // if(bytes_received==SOCKET_ERROR) {
  //   printf("recvfrom failed with error %d\n", WSAGetLastError());
  //   abort();
  // }
  // Enforce sending string w/o terminating null
  assert(buf[bytes_received-1]!='\0');
  buf[bytes_received]='\0';

  printf("%s\n",buf);

  // Send acknowledgement
  // const char *ack="ack";
  // assert(SOCKET_ERROR!=sendto(sockfd,ack,strlen(ack),0,(SOCKADDR*)(&client),sizeof(struct sockaddr_in)));

}

int main(){

  initialize_winsock();

  create_socket();
  bind_socket();
  receive_data();

  getchar();

  return 0;

}

// AF_INET
