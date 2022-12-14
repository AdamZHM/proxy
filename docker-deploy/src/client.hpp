#ifndef __CLIENT_HPP__
#define __CLIENT_HPP__

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Client {
 public:
  int id;
  int socket_fd;
  string ip;
  string host;
  string first_line;
  string url;
  int get_socket_fd() { return this->socket_fd; }
  void set_socket_fd(int socket_fd) { this->socket_fd = socket_fd; }

  int createClient(const char* hostname, const char* port);

  void close_socket_fd() { close(socket_fd); }
};

#endif