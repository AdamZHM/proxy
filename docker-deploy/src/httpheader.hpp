#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
// parse request info into first line in log format
class HttpHeader {
 private:
  std::string first_line;
  std::string method;  // ("get"/"post"/"connect")
  std::string url;     // the destination web path
  std::string host;  // the destination web server name// using it as pk to find
                     // in cache
  std::string cache_control;
  std::string port;
  std::string if_modified_since;
  std::string if_node_match;
  int content_length;

 public:
  explicit HttpHeader(const char* buffer);
  char* parseInLogFormat(const char* client_ip);
  const char* get_host() { return host.c_str(); }
  const char* get_method() { return method.c_str(); }
  const char* get_port() { return port.c_str(); }
  const char* get_url() { return url.c_str(); }
  const char* get_first_line() { return first_line.c_str(); }
  int get_content_length() {return content_length;}
};
