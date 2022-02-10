#ifndef __HELPER_HPP__
#define __HELPER_HPP__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <netdb.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <vector>
#include <algorithm>

int check_recv(int byte_count);
int check_send(int byte_count);

#endif