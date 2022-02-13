#include "server.hpp"

#include "client.hpp"
#include "httpheader.hpp"

using namespace std;

int Server::create_server(const char *port) {
  int status;
  int socket_fd;
  struct addrinfo host_info;        // hints
  struct addrinfo *host_info_list;  // results
  const char *hostname = "vcm-24373.vm.duke.edu";
  // const char *hostname = "vcm-24353.vm.duke.edu";
  // const char *port     = "12345";

  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags = AI_PASSIVE;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  }

  // use os randomly assigned port.
  if (port == NULL) {
    struct sockaddr_in *addr = (struct sockaddr_in *)(host_info_list->ai_addr);
    addr->sin_port = 0;
  }

  socket_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  }  // if

  int yes = 1;
  status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot bind socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  }  // if

  status = listen(socket_fd, 100);
  if (status == -1) {
    cerr << "Error: cannot listen on socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  }  // if

  // cout << "Waiting for connection on port " << port << endl;
  freeaddrinfo(host_info_list);
  this->socket_fd = socket_fd;
  return socket_fd;
}

char *Server::accept_connection() {
  struct sockaddr_in *socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  int client_connection_fd = accept(
      this->socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
  if (client_connection_fd == -1) {
    cerr << "Error: cannot accept connection on socket" << endl;
    exit(EXIT_FAILURE);
  }  // if
  struct sockaddr_in *pV4Addr = (struct sockaddr_in *)&socket_addr;
  struct in_addr ipAddr = pV4Addr->sin_addr;
  char *ip = (char *)malloc(INET_ADDRSTRLEN);
  inet_ntop(AF_INET, &ipAddr, ip, INET_ADDRSTRLEN);
  this->set_client_connection_fd(client_connection_fd);
  return ip;
}

void Server::close_client_connection_fd() { close(this->client_connection_fd); }

void Server::deal_with_get_request(const char *host, char *buffer) {
  Client proxy_as_client;
  int buffer_size = 50000;
  char response[buffer_size];
  memset(response, 0, buffer_size);
  cout << buffer << endl;
  proxy_as_client.createClient(host, "80");
  check_send(send(proxy_as_client.get_socket_fd(), buffer, buffer_size, 0));
  check_recv(recv(proxy_as_client.get_socket_fd(), response, buffer_size, 0));
  send(this->get_client_connection_fd(), response, buffer_size, 0);
  proxy_as_client.close_socket_fd();
  cout << response << endl;
  this->close_client_connection_fd();
}

void Server::deal_with_connect_request(const char *host, const char *port,
                                       char *buffer) {
  std::string msg("HTTP/1.1 200 OK\r\n\r\n");
  check_send(
      send(this->get_client_connection_fd(), msg.c_str(), sizeof(msg), 0));
  Client proxy_as_client;
  int buffer_size = 50000;
  char response[buffer_size];
  memset(response, 0, buffer_size);
  cout << buffer << endl;
  proxy_as_client.createClient(host, port);
  std::cout << "_________THIS IS CONNECT REQUEST_______\n"
            << buffer << std::endl;
  fd_set readfds;
  int max_fd =
      proxy_as_client.get_socket_fd() > this->get_client_connection_fd()
          ? proxy_as_client.get_socket_fd() + 1
          : this->get_client_connection_fd() + 1;
  // check_send(send(proxy_as_client.get_socket_fd(), buffer, buffer_size, 0));
  // cout << proxy_as_client.get_socket_fd() << " " << host <<  endl;
  // check_recv(recv(proxy_as_client.get_socket_fd(), response,
  //                         buffer_size, 0));
  // cout << response << endl;

  while (true) {
    FD_ZERO(&readfds);
    FD_SET(proxy_as_client.get_socket_fd(), &readfds);
    FD_SET(this->get_client_connection_fd(), &readfds);
    select(max_fd, &readfds, NULL, NULL, NULL);
    int signal = 0;
    if (FD_ISSET(proxy_as_client.get_socket_fd(), &readfds)) {
      if (check_recv(recv(proxy_as_client.get_socket_fd(), response,
                          buffer_size, 0)) > 0) {
        cout << "response: " << endl;
        cout << response << endl;
        check_send(
            send(this->get_client_connection_fd(), response, buffer_size, 0));
      } else {
        proxy_as_client.close_socket_fd();
      }
    }
    if (FD_ISSET(this->get_client_connection_fd(), &readfds)) {
      if (check_recv(
              recv(this->get_client_connection_fd(), buffer, buffer_size, 0))) {
        cout << "request: " << endl;
        cout << buffer << endl;
        check_send(
            send(proxy_as_client.get_socket_fd(), buffer, buffer_size, 0));
      } else {
        this->close_client_connection_fd();
      }
    }
    memset(buffer, 0, buffer_size);
    memset(response, 0, buffer_size);
  }
}

void Server::handle_request() {
  char *client_ip;
  int buffer_size = 50000;
  char buffer[buffer_size];
  memset(buffer, 0, sizeof(buffer));
  client_ip = this->accept_connection();
  if (recv(this->get_client_connection_fd(), buffer, buffer_size, 0) == -1) {
    this->close_client_connection_fd();
    std::cout << "_________RETURN_______\n" << std::endl;
    return;
  }
  std::cout << "_________THIS IS USER REQUEST_______\n" << buffer << std::endl;
  HttpHeader httpHeader(buffer);
  const char *host = httpHeader.get_host();
  char *method = httpHeader.get_method();
  char *port = httpHeader.get_port();
  // std::string method1("CONNECT");
  // char* m_array = (char*)malloc(method1.length());
  // std::strcpy(m_array, method1.c_str());

  if (strcmp(method, "GET") == 0) {
    std::cout << "_________THIS IS GET______\n" << std::endl;
    this->deal_with_get_request(host, buffer);
  } else if (strcmp(method, "POST") == 0) {
    this->deal_with_post_request(host, buffer);
  } else if (strcmp(method, "CONNECT") == 0) {  //
    std::cout << "_________THIS IS CONNECT______\n" << std::endl;
    this->deal_with_connect_request(host, port, buffer);
  } else {
    std::cout << "_________THIS IS NOTHING______\n" << std::endl;
  }
}

// int Server::data_from_client(){
//   int buffer_size = 10000;
//   char buffer[buffer_size];
//   memset(buffer, 0, buffer_size);
//   int all_bytes_received;
//   while (1){
//     int byte_received = recv(this->client_connection_fd, buffer,
//     buffer_size, 0); if (byte_received < 0){
//       cerr << "Error: cannot recv data from client" << endl;
//       exit(EXIT_FAILURE);
//     }
//     else if(byte_received == 0){
//       return all_bytes_received;
//     }
//     else{
//       all_bytes_received += byte_received;
//     }
//   }
// }