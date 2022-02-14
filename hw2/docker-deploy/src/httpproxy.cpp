#include "client.hpp"
#include "helper.hpp"
#include "httpheader.hpp"
#include "server.hpp"
#include "LRUCache.hpp"

using namespace std;

int main(int argc, char* argv[]) {
  const char* port = "12345";

  // LRUCache lruCache(2);
  // string url1 = "www.google.com";
  // ResponseHead res1 = ResponseHead();
  // lruCache.put(url1, res1);
  // string url2 = "www.baidu.com";
  // ResponseHead res2 = ResponseHead();
  // lruCache.put(url2, res2);
  // string url3 = "www.qq.com";
  // ResponseHead res3 = ResponseHead();
  // lruCache.put(url3, res3);



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