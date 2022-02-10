#include "server.hpp"
#include "helper.hpp"
#include "client.hpp"

using namespace std;

int main(int argc, char * argv[]){
  const char * port = "12345";
  int buffer_size = 10000;

  Server proxy_server;
  proxy_server.create_server(port);

  // char* client_ip = proxy_server.accept_connection();
  char* client_ip;
  Client proxy_as_client;
  char buffer[buffer_size];
  char response[buffer_size];
  memset(buffer, 0, sizeof(buffer));
  memset(response, 0, sizeof(response));
  client_ip = proxy_server.accept_connection();

  fd_set readfds;
  FD_ZERO(&readfds);
  while (1){
    FD_ZERO(&readfds);
    FD_SET(proxy_server.get_client_connection_fd_vector()[0], &readfds);
    select(proxy_server.get_client_connection_fd_vector()[0] + 1, &readfds, NULL, NULL, NULL);
    if (FD_ISSET(proxy_server.get_client_connection_fd_vector()[0], &readfds)){
      cout << client_ip << endl;
      check_recv(recv(proxy_server.get_client_connection_fd_vector()[0], buffer, sizeof(buffer), 0));

      cout << buffer << endl;

      proxy_as_client.createClient("info.cern.ch", "80");

      check_send(send(proxy_as_client.get_socket_fd(), buffer, sizeof(buffer), 0));


      check_recv(recv(proxy_as_client.get_socket_fd(), response, sizeof(response), 0));
      
      cout << response << endl;

      send(proxy_server.get_client_connection_fd_vector()[0], response, sizeof(response), 0);
      memset(buffer, 0, sizeof(buffer));
      memset(response, 0, sizeof(response));
      proxy_as_client.close_socket_fd();
      // free(client_ip);
    }
  }
  proxy_server.close_socket_fd();
  proxy_server.close_client_connection_fd_vector();
  return EXIT_SUCCESS;
}