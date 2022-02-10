#ifndef __CLIENT_HPP__
#define __CLIENT_HPP__

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

using namespace std;

class Client{
private:
  int socket_fd;
public:
  int get_socket_fd(){return this->socket_fd;}
  void set_socket_fd(int socket_fd){this->socket_fd = socket_fd;}

  int createClient(const char * hostname, const char * port);

  void close_socket_fd(){close(socket_fd);}
};


#endif