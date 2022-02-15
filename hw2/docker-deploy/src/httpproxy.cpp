#include "client.hpp"
#include "helper.hpp"
#include "server.hpp"

using namespace std;

mutex mtx;

void thread_task(Server *s, Client *o) {
  s->handle_request(o);
  delete (o);
  delete (s);
}

int main(int argc, char *argv[]) {
  const char *port = "12345";
  int socket_fd = create_server(port);

  while (1) {
    Server *proxy_server = new Server();
    int client_fd;
    string client_ip = proxy_server->accept_connection(socket_fd, &client_fd);
    cout << client_fd << endl;

    Client *client = new Client();
    // Client *client2 = new Client();
    client->set_socket_fd(client_fd);

    thread t(thread_task, proxy_server, client);
    t.detach();
  }
  // proxy_server->close_socket_fd();
  close(socket_fd);
  return EXIT_SUCCESS;
}
