#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "timestamp.hpp"

class ResponseHead {
 private:
  std::string response;
  std::string status_code;
  std::string cache_control;
  TimeStamp date;  // response time
  std::string etag;
  TimeStamp last_modified;
  TimeStamp expires;  // expire time
  int max_age;
  std::string transfer_encoding;

 public:
  ResponseHead()
      : response(""),
        status_code(""),
        cache_control(""),
        date(""),
        etag(""),
        last_modified(""),
        expires(""),
        max_age(-1) {}
};