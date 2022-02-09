#include "server.hpp"
#include "helper.hpp"
#include "client.hpp"

using namespace std;

int main(int argc, char * argv[]){
  const char * port = "12345";
  int buffer_size = 10000;

  Server proxy_server;
  proxy_server.create_server(port);
  proxy_server.accept_connection();
  char buffer[buffer_size] = {'\0'};
  memset(buffer, 0, sizeof(buffer));

  check_recv(recv(proxy_server.get_client_connection_fd_vector()[0], buffer, sizeof(buffer), 0));

  cout << buffer << endl << endl;

  Client proxy_as_client;
  proxy_as_client.createClient("vcm-24353.vm.duke.edu", "8000");

  cout << proxy_as_client.get_socket_fd() << endl;

  check_send(send(proxy_as_client.get_socket_fd(), buffer, sizeof(buffer), 0));

  char response[buffer_size];
  memset(response, 0, sizeof(response));

  check_recv(recv(proxy_as_client.get_socket_fd(), response, sizeof(response), 0));
  cout << response << endl;

  send(proxy_server.get_client_connection_fd_vector()[0], response, sizeof(response), 0);

  
  return EXIT_SUCCESS;
}