#include "server.hpp"

using namespace std;

int Server::create_server(const char *port){
  int status;
  int socket_fd;
  struct addrinfo host_info; //hints
  struct addrinfo *host_info_list; //results
  const char *hostname = "vcm-24353.vm.duke.edu";
  const char *port     = "12345";

  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family   = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags    = AI_PASSIVE;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  } 

  // use os randomly assigned port.
  if (port == NULL){
    struct sockaddr_in* addr = (struct sockaddr_in *)(host_info_list->ai_addr);
    addr->sin_port = 0;
  }

  socket_fd = socket(host_info_list->ai_family, 
		     host_info_list->ai_socktype, 
		     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  } //if

  int yes = 1;
  status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot bind socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  } //if

  status = listen(socket_fd, 100);
  if (status == -1) {
    cerr << "Error: cannot listen on socket" << endl; 
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  } //if

  // cout << "Waiting for connection on port " << port << endl;
  freeaddrinfo(host_info_list);
  this->socket_fd = socket_fd;
  return socket_fd;
}

void Server::accept_connection(){
  struct sockaddr_in * socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  int client_connection_fd = accept(this->socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
  if (client_connection_fd == -1) {
    cerr << "Error: cannot accept connection on socket" << endl;
    exit(EXIT_FAILURE);
  } //if
  this->add_to_client_connection_fd_vector(client_connection_fd);
}