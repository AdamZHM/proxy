#include "client.hpp"
#include "helper.hpp"
#include "server.hpp"

using namespace std;

mutex mtx;


void thread_task(Server* s, Client* o) {
  s->handle_request(o);
  delete(o);
}
// void do_thread(Server * server){
//   server->handle_request();
//   delete(server);
// }

int main(int argc, char *argv[]) {
  const char *port = "12345";
  Server *proxy_server = new Server();
  proxy_server->create_server(port);

  while (1) {
    int client_fd;
    string client_ip = proxy_server->accept_connection(&client_fd);
    cout << client_fd << endl;
   
    Client *client = new Client();
    // Client *client2 = new Client();
    client->set_socket_fd(client_fd);
  
    // proxy_server.handle_request(client1);
    // proxy_server.handle_request(client2);
    thread(thread_task, proxy_server, client);

  }
  proxy_server->close_socket_fd();
  return EXIT_SUCCESS;
}
