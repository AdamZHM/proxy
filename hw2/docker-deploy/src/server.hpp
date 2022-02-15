#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
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
#include <thread>         // std::thread
#include <mutex>

#include "LRUCache.hpp"
#include "client.hpp"
#include "helper.hpp"
#include "httpheader.hpp"

using namespace std;
// using static initializetion

class Server {
 private:
  // int socket_fd;
  const char* port;
  int client_connection_fd;

 public:
  // int get_socket_fd() { return this->socket_fd; }
  // void set_socket_fd(int socket_fd) { this->socket_fd = socket_fd; }

  const char* get_port() { return this->port; }
  void set_port(const char* port) { this->port = port; }

  int get_client_connection_fd() { return client_connection_fd; }
  void set_client_connection_fd(int fd) { this->client_connection_fd = fd; }

  // static int create_server(const char* port);
  string accept_connection(int socket_fd, int* client_connection_fd);

  void handle_request(Client* client);
  void deal_with_get_request(Client proxy_as_client, const char* url,
                             char* buffer, Client* client);
  void deal_with_post_request(Client proxy_as_client, const char* url,
                              char* buffer, Client* client) {
    deal_with_get_request(proxy_as_client, url, buffer, client);
  }
  void deal_with_connect_request(Client proxy_as_client, Client* client);

  // void close_socket_fd() { close(socket_fd); }
  void close_client_connection_fd();

  bool revalidation(ResponseHead resp, Client proxy_as_client, Client* client);
  bool ifExpired(ResponseHead resp);
};

#endif