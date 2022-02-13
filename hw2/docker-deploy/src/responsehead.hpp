#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "timestamp.hpp"

std::vector<char> convertCharStarToVecChar(const char * buffer, int size) {
  std::vector<char> ans;
  for (int i = 0; i < size; i++) {
    ans.push_back(buffer[i]);
  }
  return ans;
}

void changeHeaderToLower(std::string & line, std::string & temp) {
  temp = line;
  std::transform(temp.begin(), temp.end(), temp.begin(), [](unsigned char c) {
    return std::tolower(c);
  });
}

class ResponseHead {
 public:
  std::vector<char> response;
  std::string status;  // a whole line contains HTTP/1.1 status_cdoe \r\n
  bool if_cache_control;
  TimeStamp date;  // response time
  std::string etag;
  TimeStamp last_modified;
  TimeStamp expires;  // expire time
  int max_age;
  bool if_chunked;
  bool if_no_store;
  bool if_no_cache;
  bool if_must_revalidate;

 public:
  ResponseHead() :
      response(std::vector<char>()),
      status(""),
      if_cache_control(false),
      date(TimeStamp()),
      etag(""),
      last_modified(TimeStamp()),
      expires(TimeStamp()),
      max_age(-1),
      if_chunked(false),
      if_no_store(false),
      if_no_cache(false),
      if_must_revalidate(false) {}

  void initResponse(const char * buffer, int size) {
    response = convertCharStarToVecChar(buffer, size);
    std::string bufferStr(buffer);
    std::istringstream ss(buffer);
    std::string line;
    while (getline(ss, line)) {
      if(line.find('\r')==0) break;
      std::string lowerLine;
      changeHeaderToLower(line, lowerLine);
      if (lowerLine.find("http/1.1") == 0) {
        status = line;
      }
      else if (lowerLine.find("cache-control") == 0) {
        if_cache_control = true;
        if (line.find("no-store") != std::string::npos) {
          if_no_store = true;
        }
        if (line.find("no-cache") != std::string::npos) {
          if_no_cache = true;
        }
        if (line.find("must-revalidate") != std::string::npos) {
          if_must_revalidate = true;
        }
        if (line.find("max-age") != std::string::npos) {
          std::string temp(line.substr(line.find("max-age") + 9));
          max_age = std::stoi(temp);
        }
      }
      else if (lowerLine.find("date") == 0) {
        int startIdx = 6;
        date = TimeStamp(line.substr(startIdx));
      }
      else if (lowerLine.find("etag") == 0) {  // include /r/n
        int startIdx = 6;
        etag = line.substr(startIdx);
      }
      else if (lowerLine.find("last-modified") == 0) {
        int startIdx = 15;
        last_modified = TimeStamp(line.substr(startIdx));
      }
      else if (lowerLine.find("expires") == 0) {
        int startIdx = 9;
        expires = TimeStamp(line.substr(startIdx));
      }
      else if (lowerLine.find("transfer-encoding") == 0 && lowerLine.find("chunked")) {
        if_chunked = true;
      }
    }
  }
};