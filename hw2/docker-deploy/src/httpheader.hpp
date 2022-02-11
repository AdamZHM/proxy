#include <cstdlib>
#include <string>
//parse request info into first line in log format
class HttpHeader {
 private:
  char * method;  // ("get"/"post"/"connect")
  char * url;     // the destination web path
  char * host;    // the destination web server name
 public:
  HttpHeader() : method(nullptr), url(nullptr), host(nullptr){};
  explicit HttpHeader( const char * buffer);
  char * parseInLogFormat( const char * client_ip);
  ~HttpHeader(){
    delete method;
    delete url;
    delete host;
  }
  char * get_host(){return host;}
};
