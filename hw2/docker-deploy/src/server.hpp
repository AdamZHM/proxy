#ifndef __SERVER_HPP__
#define __SERVER_HPP__

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

class Server{
private:
  int socket_fd;
  const char *port;
  vector<int> client_connection_fd_vector;
public:
  int get_socket_fd(){return this->socket_fd;}
  void set_socket_fd(int socket_fd){this->socket_fd = socket_fd;}

  const char* get_port(){return this->port;}
  void set_port(const char *port){this->port = port;}

  vector<int> get_client_connection_fd_vector(){return client_connection_fd_vector;}
  void add_to_client_connection_fd_vector(int fd){this->client_connection_fd_vector.push_back(fd);}

  int create_server(const char *port);
  void accept_connection();
};

#endif