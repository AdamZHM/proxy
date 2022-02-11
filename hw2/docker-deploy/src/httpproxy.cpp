#include "client.hpp"
#include "helper.hpp"
#include "httpheader.hpp"
#include "server.hpp"

using namespace std;

int main(int argc, char* argv[]) {
  const char* port = "12345";

  Server proxy_server;
  proxy_server.create_server(port);

  char* client_ip;
  Client proxy_as_client;

  client_ip = proxy_server.accept_connection();
  proxy_server.deal_with_get_request(proxy_server);
  // client_ip = proxy_server.accept_connection();
  // proxy_server.deal_with_get_request(proxy_server);

  proxy_server.close_socket_fd();
  return EXIT_SUCCESS;
}