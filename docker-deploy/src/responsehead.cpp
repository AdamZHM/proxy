#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "helper.hpp"

class ResponseHead {
 public:
  std::vector<char> response;
  std::string status;  // a whole line contains HTTP/1.1 status_cdoe \r\n
  bool if_cache_control;
  TimeStamp date;  // response time
  std::string etag;
  std::string last_modified;
  std::string expires;  // expire time
  int content_length;
  int max_age;
  bool if_chunked;
  bool if_no_store;
  bool if_no_cache;
  bool if_must_revalidate;

 public:
  ResponseHead()
      : response(std::vector<char>()),
        status(""),
        if_cache_control(false),
        date(TimeStamp()),
        etag(""),
        last_modified(""),
        expires(""),
        content_length(-1),
        max_age(-1),
        if_chunked(false),
        if_no_store(false),
        if_no_cache(false),
        if_must_revalidate(false) {}

  void initResponse(const char *buffer, int size) {
    std::cout << buffer << std::endl;
    response = convertCharStarToVecChar(buffer, size);
    std::string bufferStr(buffer);
    std::istringstream ss(buffer);
    std::string line;
    while (getline(ss, line)) {
      if (line.find('\r') == 0) break;
      std::string lowerLine;
      changeHeaderToLower(line, lowerLine);
      if (lowerLine.find("http/1.1") == 0) {
        status = line.substr(0, line.find("\r"));
      } else if (lowerLine.find("cache-control:") != std::string::npos) {
        if_cache_control = true;
        if (line.find("no-store") != std::string::npos || line.find("private") != std::string::npos) {
          if_no_store = true;
        }
        if (line.find("no-cache") != std::string::npos) {
          if_no_cache = true;
        }
        if (line.find("must-revalidate") != std::string::npos) {
          if_must_revalidate = true;
        }
        if (line.find("max-age") != std::string::npos) {
          std::string temp(line.substr(line.find("max-age") + 8));
          max_age = atoi(temp.c_str());
          std::cout << max_age << std::endl;
        }
      } else if (lowerLine.find("date:") == 0) {
        int startIdx = 6;
        date = TimeStamp(line.substr(startIdx));
      } else if (lowerLine.find("etag:") == 0) { 
        int startIdx = 6;
        etag = line.substr(startIdx, line.find("\r") - 6);
      } else if (lowerLine.find("last-modified:") == 0) {
        int startIdx = 15;
        last_modified = line.substr(startIdx, line.find("\r") - 15);
      } else if (lowerLine.find("expires:") != std::string::npos && expires == "") {
        int startIdx = lowerLine.find("expires") + 9;
        expires = line.substr(startIdx);
      } else if (lowerLine.find("transfer-encoding") == 0 &&
                 lowerLine.find("chunked")) {
        if_chunked = true;
      } else if (lowerLine.find("content-length") == 0) {
        content_length = atoi(line.substr(line.find(" ") + 1).c_str());
        // std::cout << content_length << std::endl;
      }
    }
  }

  void appendResponse(const char *buffer, int size) {
    for (int i = 0; i < size; i++) {
      response.push_back(buffer[i]);
    }
  }

  int totalLength() {
    std::string str_response(response.data());
    int headerLength = str_response.find("\r\n\r\n") + 4;
    return headerLength + content_length;
  }

  // this will print out
  // ID: not cacheable becauseREASON
  // ID: cached, expires atEXPIRES
  // ID: cached, but requires re-validation
  void printCacheReponseToGetReq(std::ofstream &fout, Client *client) {
    if (this->if_chunked) {
      fout << client->id << ": not cacheable because the data is chunked"
           << endl;
    } else if (this->if_no_store) {
      fout << client->id << ": not cacheable because Cache-control: no_store"
           << endl;
    }
    // first check if expires is empty or not then check max_age
    else if (this->expires != "") {
      std::cout << "________this expires " << this->expires << endl;
      TimeStamp expireTime(expires);
      time_t t = mktime(expireTime.get_t());
      struct tm *tm = gmtime(&t);
      tm->tm_year -= 1900;
      tm->tm_wday -= 1;
      std::string time = std::string(asctime(tm));
      time.pop_back();
      fout << client->id << ": cached, expires at " << time << endl;
    } else if (this->max_age != -1) {
      time_t t = std::time(0);  // get time now
      t += max_age;
      struct tm *tm = gmtime(&t);
      std::string time = std::string(asctime(tm));
      time.pop_back();
      fout << client->id << ": in cache, expires at " << time << endl;
    } else if (this->if_cache_control == false || this->if_no_cache == true ||
               this->if_must_revalidate == true || this->max_age == 0) {
      fout << client->id << ": cached, but requires re-validation " << endl;
    }
  }
};
