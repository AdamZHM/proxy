#include "server.hpp"

#include "client.hpp"
#include "httpheader.hpp"
#include "responsehead.hpp"

using namespace std;

int Server::create_server(const char *port) {
  int status;
  int socket_fd;
  struct addrinfo host_info;        // hints
  struct addrinfo *host_info_list;  // results
  const char *hostname = "vcm-24353.vm.duke.edu";
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

string Server::accept_connection() {
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
  string ans(ip);
  free(ip);
  return ans;
}

void Server::close_client_connection_fd() { close(this->client_connection_fd); }

void Server::deal_with_get_request(const char *host, const char *port,
                                   char *buffer) {
  Client proxy_as_client;
  int buffer_size = 65536;
  char response[buffer_size];
  memset(response, 0, buffer_size);
  cout << buffer << endl;
  proxy_as_client.createClient(host, port);
  if (send(proxy_as_client.get_socket_fd(), buffer, buffer_size, 0) < 0) {
    cerr << "Error : send data to server in GET" << endl;
    exit(EXIT_FAILURE);
  }
  int bytes_recv =
      recv(proxy_as_client.get_socket_fd(), response, buffer_size, 0);
  if (bytes_recv == 0) {
    cout << "Server shut down the connection" << endl;
    proxy_as_client.close_socket_fd();
    this->close_client_connection_fd();
    return;
  } else if (bytes_recv < 0) {
    cerr << "Error: recv data from server" << endl;
    exit(EXIT_FAILURE);
  }
  
  ResponseHead rph;
  rph.initResponse(response, bytes_recv);

  if (send(this->get_client_connection_fd(), response, buffer_size, 0) < 0) {
    cerr << "Error in send data to client in GET" << endl;
    exit(EXIT_FAILURE);
  }
  cout << response << endl;
  proxy_as_client.close_socket_fd();
  this->close_client_connection_fd();
}

void Server::deal_with_connect_request(const char *host, const char *port) {
  const char *msg = "HTTP/1.1 200 OK\r\n\r\n";
  if (send(this->get_client_connection_fd(), msg, 19, 0) < 0) {
    cerr << "Error send data in connect " << endl;
    exit(EXIT_FAILURE);
  }
  Client proxy_as_client;
  int buffer_size = 65536;
  proxy_as_client.createClient(host, port);
  vector<int> fd_vector = {proxy_as_client.get_socket_fd(),
                      this->get_client_connection_fd()};
  fd_set readfds;
  int max_fd =
      fd_vector[0] > fd_vector[1] ? fd_vector[0] + 1 : fd_vector[1] + 1;
  while (1) {
    FD_ZERO(&readfds);
    for (int i = 0; i < 2; i++) {
      FD_SET(fd_vector[i], &readfds);
    }
    int status = select(max_fd, &readfds, NULL, NULL, NULL);
    if (status == -1) {
      cerr << "Error in select" << endl;
      exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 2; i++) {
      char buffer[buffer_size];
      memset(buffer, 0, buffer_size);
      if (FD_ISSET(fd_vector[i], &readfds)) {
        int bytes_recv = recv(fd_vector[i], buffer, buffer_size, 0);
        if (bytes_recv == 0) {
          this->close_client_connection_fd();
          proxy_as_client.close_socket_fd();
          return;
        } else if (bytes_recv < 0) {
          cerr << "Error : recv data error" << endl;
          exit(EXIT_FAILURE);
        }
        buffer[bytes_recv] = '\0';

        if (send(fd_vector[(i + 1) % 2], buffer, bytes_recv, 0) < 0) {
          cerr << "Error : send data error" << endl;
          exit(EXIT_FAILURE);
        }
      }
    }
  }
}

void Server::handle_request() {
  string client_ip;
  int buffer_size = 65536;
  char buffer[buffer_size];
  memset(buffer, 0, sizeof(buffer));
  client_ip = this->accept_connection();
  int byte_recv =
      recv(this->get_client_connection_fd(), buffer, buffer_size, 0);
  if (byte_recv == -1) {
    this->close_client_connection_fd();
    std::cout << "_________RETURN_______\n" << std::endl;
    return;
  }
  buffer[byte_recv] = '\0';
  std::cout << "_________THIS IS USER REQUEST_______\n" << buffer << std::endl;
  HttpHeader httpHeader(buffer);
  const char *host = httpHeader.get_host();
  const char *method = httpHeader.get_method();
  const char *port = httpHeader.get_port();

  if (strcmp(method, "GET") == 0) {
    std::cout << "_________THIS IS GET______\n" << std::endl;
    this->deal_with_get_request(host, port, buffer);
  } else if (strcmp(method, "POST") == 0) {
    this->deal_with_post_request(host, port, buffer);
  } else if (strcmp(method, "CONNECT") == 0) {  //
    std::cout << "_________THIS IS CONNECT______\n" << std::endl;
    this->deal_with_connect_request(host, port);
  } else {
    std::cout << "_________THIS IS NOTHING______\n" << std::endl;
  }
}
