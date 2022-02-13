#include "client.hpp"
#include "helper.hpp"
#include "httpheader.hpp"
#include "server.hpp"
#include "timestamp.hpp"

using namespace std;

int main(int argc, char* argv[]) {
  const char* port = "12345";


  //test
  std::string s1 = "Sun, 13 Feb 2022 15:51:34 GMT";
  // std::string s2 = "Expires: Sun, 13 Feb 2022 15:59:34 GMT";
  TimeStamp timestamp1(s1);
  // TimeStamp timestamp2(s2);
  // cout << timestamp1.timediff(timestamp2) << endl;

  Server proxy_server;
  proxy_server.create_server(port);
  // client_ip = proxy_server.accept_connection();
  // proxy_server.deal_with_get_request(proxy_server);
  while (1) {
    proxy_server.handle_request();
  }

  proxy_server.close_socket_fd();
  return EXIT_SUCCESS;
}