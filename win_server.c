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
#include <iphlpapi.h> // IP Helper API // GetAdaptersAddresses() GetAdaptersInfo() 

#include <shlobj.h> // ShellExecute()
#include <synchapi.h> // Sleep()

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "./def.h"

// https://docs.microsoft.com/en-us/cpp/preprocessor/comment-c-cpp
// #pragma comment(lib, "Ws2_32.lib")   // C:\Windows\System32\ws2_32.dll
// #pragma comment(lib, "IPHLPAPI.lib") // C:\Windows\System32\IPHLPAPI.DLL

#define MALLOC(x) HeapAlloc(GetProcessHeap(),0,(x))
#define FREE(x) HeapFree(GetProcessHeap(),0,(x))

SOCKET sockfd=INVALID_SOCKET;
IP_ADDRESS_STRING local_ip={};

char buf[SZ+1]={};

void openlnk(){
  // ShellExecuteA()
  ShellExecute(NULL,"open",buf,NULL,NULL,SW_SHOWNORMAL);
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

bool all_zero(const IP_ADDRESS_STRING *const pip){
  static_assert(16==sizeof(((PIP_ADDRESS_STRING)NULL)->String));
  static_assert(16==sizeof(pip->String));
  return(
    '\0'==pip->String[0] &&
      0 ==strncmp(pip->String,pip->String+1,15)
  );
}

void get_local_ip(){

  ULONG required=0;
  assert(ERROR_BUFFER_OVERFLOW==GetAdaptersInfo(NULL,&required));
  // printf("%lu\n",required);
  const ULONG bak=required;
  PIP_ADAPTER_INFO pAdapterInfo=(IP_ADAPTER_INFO*)MALLOC(required);
  assert(ERROR_SUCCESS==GetAdaptersInfo(pAdapterInfo,&required));
  assert(bak==required);

  for(const IP_ADAPTER_INFO *p=pAdapterInfo;p!=NULL;p=p->Next){

    // Filter out adapters without gateway (1/2)
    if(0==strcmp("0.0.0.0",p->GatewayList.IpAddress.String))
      continue;

    // printf("%lu ",p->ComboIndex); // Reserved
    // printf("%s ",p->AdapterName); // GUID
    // printf("%-60s ",p->Description);
    printf("%s ",p->Description);

    // MAC
    // printf("%u ",p->AddressLength);
    assert(6==p->AddressLength);
    // https://stackoverflow.com/a/40558754/
    // No difference between field width & precision for integer
    // for(UINT i=0;i<p->AddressLength;++i){printf((i==0)?"%.2X":"-%.2X",(UCHAR)(p->Address[i]));}putchar(' ');
    for(UINT i=0;i<p->AddressLength;++i){printf((i==0)?"%02X":"-%02X",(UCHAR)(p->Address[i]));}putchar(' ');

    printf("%lu ",p->Index);
    // printf("%u ",p->Type);
    assert(p->Type==MIB_IF_TYPE_ETHERNET||p->Type==IF_TYPE_IEEE80211);
    // printf("%u ",p->DhcpEnabled);

    // printf("%p ",p->CurrentIpAddress); // Reserved
    assert(NULL==p->CurrentIpAddress); // Reserved

    // Adapters w/ multiple IP addresses
    // printf("ip ");
    // for(const IP_ADDR_STRING *pp=&(p->IpAddressList);pp!=NULL;pp=pp->Next)printf("( %s %s %lu ) ",
    //   pp->IpAddress.String,
    //   pp->IpMask.String,
    //   pp->Context
    // );

    // Adapters w/ one IP address each
    printf("ip ");
    assert(NULL==p->IpAddressList.Next);
    printf("( %s %s %lu ) ",
      p->IpAddressList.IpAddress.String,
      p->IpAddressList.IpMask.String,
      p->IpAddressList.Context
    );
    // Filter out adapters without gateway (2/2)
    if(0!=strcmp("0.0.0.0",p->GatewayList.IpAddress.String)){
      assert(all_zero(&local_ip));
      assert(p->IpAddressList.IpAddress.String[15]=='\0');
      strcpy(local_ip.String,p->IpAddressList.IpAddress.String);
    }

    printf("gw ");
    for(const IP_ADDR_STRING *pp=&(p->GatewayList);pp!=NULL;pp=pp->Next){
      assert(0==strcmp("255.255.255.255",pp->IpMask.String));
      printf("[ %s %lu ] ",
        pp->IpAddress.String,
        pp->Context
      );
    }

    printf("dhcpfrom ");
    for(const IP_ADDR_STRING *pp=&(p->DhcpServer);pp!=NULL;pp=pp->Next)printf("{ %s %s %lu } ",
      pp->IpAddress.String,
      pp->IpMask.String,
      pp->Context
    );

    assert(
                  !(p->     HaveWins                 )
      &&     NULL==(p->  PrimaryWinsServer.Next      )
      && all_zero(&(p->  PrimaryWinsServer.IpAddress))
      && all_zero(&(p->  PrimaryWinsServer.IpMask   ))
      &&        0==(p->  PrimaryWinsServer.Context   )
      &&     NULL==(p->SecondaryWinsServer.Next      )
      && all_zero(&(p->SecondaryWinsServer.IpAddress))
      && all_zero(&(p->SecondaryWinsServer.IpMask   ))
      &&        0==(p->SecondaryWinsServer.Context   )
    );
    // printf("%d ",p->HaveWins);
    // printf("WINSp ");
    // for(const IP_ADDR_STRING *pp=&(p->PrimaryWinsServer);pp!=NULL;pp=pp->Next)printf("{ %s %s %lu } ",
    //   pp->IpAddress.String,
    //   pp->IpMask.String,
    //   pp->Context
    // );
    // printf("WINSs ");
    // for(const IP_ADDR_STRING *pp=&(p->SecondaryWinsServer);pp!=NULL;pp=pp->Next)printf("{ %s %s %lu } ",
    //   pp->IpAddress.String,
    //   pp->IpMask.String,
    //   pp->Context
    // );

    printf("%lld ",p->LeaseObtained);
    printf("%lld ",p->LeaseExpires);

    printf("\n");

  }

  // Why is a single FREE() enough?
  FREE(pAdapterInfo);

  // On Windows XP and later:  Use the GetAdaptersAddresses function instead of GetAdaptersInfo.
  // GetAdaptersAddresses(AF_INET,
  //   GAA_FLAG_SKIP_UNICAST
  //   // 
  //   GAA_FLAG_SKIP_ANYCAST
  //   GAA_FLAG_SKIP_MULTICAST
  //   GAA_FLAG_SKIP_DNS_SERVER
  //   // 
  //   GAA_FLAG_INCLUDE_PREFIX
  //   // 
  //   GAA_FLAG_SKIP_FRIENDLY_NAME
  //   // 
  // ,NULL,pAddresses,&outBufLen);

}

void create_socket(){
  sockfd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
  assert(sockfd!=INVALID_SOCKET);
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
  static_assert(FALSE==0);
  if('\0'==local_ip.String[0]||0==strcmp("0.0.0.0",local_ip.String)){
    printf("offline\n");
    assert(FALSE);
  }
  #define STR(s) STR0(s)
  #define STR0(s) #s
  printf("Listening on %s:"STR(DEFAULT_PORT)" ...\n",local_ip.String);
  #undef STR
}

void receive_data(){

  ZeroMemory(buf,SZ+1);

  struct sockaddr_in client={};
  ZeroMemory(&client,sizeof(struct sockaddr_in));

  const int bytes_received=recvfrom(sockfd,buf,SZ,0/* no flags*/,(SOCKADDR*)(&client),&((int){sizeof(client)}));
  assert(
    bytes_received!=SOCKET_ERROR &&
    bytes_received>=1 &&
    bytes_received<=SZ
  );
  // WSAGetLastError()
  // Enforce sending string w/o terminating null
  assert(buf[bytes_received-1]!='\0');
  buf[bytes_received]='\0';

  // Send acknowledgement
  // const char *ack="ack";
  // assert(SOCKET_ERROR!=sendto(sockfd,ack,strlen(ack),0,(SOCKADDR*)(&client),sizeof(struct sockaddr_in)));

}

void loop(){
  do{
    receive_data();
    printf("%s\n",buf);
    openlnk();
  }while(0!=strcmp(CMD_QUIT,buf));
}

void cleanup(){
  printf("quit ...\n");
  assert(0==closesocket(sockfd));
  Sleep(500UL); // Milliseconds
  // printf("Press Enter to exit\n");
  // getchar();
}

int main(){

  initialize_winsock();

  get_local_ip();

  create_socket();
  bind_socket();

  // receive_data();
  loop();

  cleanup();
  return 0;

}
