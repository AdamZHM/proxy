#include "server.hpp"

#include "client.hpp"
#include "httpheader.hpp"

using namespace std;

int Server::create_server(const char *port) {
  int status;
  int socket_fd;
  struct addrinfo host_info;        // hints
  struct addrinfo *host_info_list;  // results
  const char *hostname = "vcm-24353.vm.duke.edu";
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

void Server::deal_with_get_request(Server proxy_server) {
  Client proxy_as_client;
  int buffer_size = 10000;
  char buffer[buffer_size];
  char response[buffer_size];
  while (1) {
    memset(buffer, 0, sizeof(buffer));
    memset(response, 0, sizeof(response));
    if (recv(proxy_server.get_client_connection_fd(), buffer, sizeof(buffer),
             0) == 0) {
      proxy_server.close_client_connection_fd();
      break;
    }
    // TODO: parse header here
    cout << buffer << endl;
    HttpHeader httpHeader(buffer);
    char *host = httpHeader.get_host();
    proxy_as_client.createClient(host, "80");
    check_send(
        send(proxy_as_client.get_socket_fd(), buffer, sizeof(buffer), 0));
    check_recv(
        recv(proxy_as_client.get_socket_fd(), response, sizeof(response), 0));
    send(proxy_server.get_client_connection_fd(), response, sizeof(response),
         0);
    proxy_as_client.close_socket_fd();
    cout << response << endl;
  }
}

void Server::deal_with_connect_request(Server proxy_server) {
  Client proxy_as_client;
  int buffer_size = 50000;
  char buffer[buffer_size];
  char response[buffer_size];
  SSL_load_error_strings();
  SSL_library_init();
  SSL_CTX *ssl_ctx = SSL_CTX_new(SSLv23_client_method());
  SSL *conn = SSL_new(ssl_ctx);
  while (1) {
    memset(buffer, 0, sizeof(buffer));
    memset(response, 0, sizeof(response));
    if (recv(proxy_server.get_client_connection_fd(), buffer, sizeof(buffer),
             0) == 0) {
      proxy_server.close_client_connection_fd();
      break;
    }
    cout << buffer << endl;
    proxy_as_client.createClient("www.google.com", "443");
    SSL_set_fd(conn, proxy_as_client.get_socket_fd());
    int err = SSL_connect(conn);
    if (err != 1) {
      abort();
    }
    SSL_write(conn, buffer, buffer_size);
    SSL_read(conn, response, sizeof(response));
    send(proxy_server.get_client_connection_fd(), response, sizeof(response),
         0);
    proxy_as_client.close_socket_fd();
    cout << response << endl;
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