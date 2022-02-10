#include "server.hpp"
#include "helper.hpp"
#include "client.hpp"

using namespace std;

int main(int argc, char * argv[]){
  const char * port = "12345";
  int buffer_size = 10000;

  Server proxy_server;
  proxy_server.create_server(port);
  char* client_ip = proxy_server.accept_connection();
  cout << client_ip << endl;
  char buffer[buffer_size] = {'\0'};
  // memset(buffer, 0, sizeof(buffer));
  check_recv(recv(proxy_server.get_client_connection_fd_vector()[0], buffer, sizeof(buffer), 0));

  cout << buffer << endl;

  Client proxy_as_client;
  proxy_as_client.createClient("info.cern.ch", "80");

  check_send(send(proxy_as_client.get_socket_fd(), buffer, sizeof(buffer), 0));

  char response[buffer_size] = {'\0'};
  memset(response, 0, sizeof(response));

  check_recv(recv(proxy_as_client.get_socket_fd(), response, sizeof(response), MSG_WAITALL));
  
  cout << response << endl;

  send(proxy_server.get_client_connection_fd_vector()[0], response, sizeof(response), 0);

  free(client_ip);
  proxy_as_client.close_socket_fd();
  proxy_server.close_socket_fd();
  proxy_server.close_client_connection_fd_vector();
  return EXIT_SUCCESS;
}