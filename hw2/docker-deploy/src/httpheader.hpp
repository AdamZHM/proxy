#include <cstdlib>
#include <string>
#include <vector>
#include <cstring>
#include <ctime>
#include <iostream>
#include <sstream>
#include <algorithm>
// parse request info into first line in log format
class HttpHeader {
 public:
  char* method;  // ("get"/"post"/"connect")
  char* url;     // the destination web path
  char* host;    // the destination web server name
 public:
  HttpHeader() : method(nullptr), url(nullptr), host(nullptr){};
  explicit HttpHeader(const char* buffer);
  char* parseInLogFormat(const char* client_ip);
  ~HttpHeader() {
    delete method;
    delete url;
    delete host;
  }
  char* get_host() { return host; }
};
