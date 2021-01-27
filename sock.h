#ifndef __SOCK_H__
#define __SOCK_H__

#include <netinet/in.h> // struct sockaddr_in

void init0(const char *const);
void send0(const char *const);
void recv0();
void end0();

#endif
