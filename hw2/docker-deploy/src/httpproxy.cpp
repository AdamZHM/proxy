#include <pthread.h>

#include "client.hpp"
#include "helper.hpp"
#include "server.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  const char *port = "12345";
  Server proxy_server;
  proxy_server.create_server(port);

  while (1) {
    int client_fd;
    string client_ip = proxy_server.accept_connection(&client_fd);
    cout << client_fd << endl;
    // mtx.lock();
    Client *client = new Client();
    client->set_socket_fd(client_fd);
    proxy_server.handle_request(client);
  }
  proxy_server.close_socket_fd();
  return EXIT_SUCCESS;
}
