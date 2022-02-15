#include <iostream>

#include "httpheader.hpp"

int main() {
  std::string test(
      "GET http://info.cern.ch/ HTTP/1.1\r\n"
      "Host: info.cern.ch\n");
  HttpHeader hh(test.c_str());
  std::string ip = "11.1.1.1.1";
  std::cout << hh.url << '\n';
  std::cout << hh.parseInLogFormat(ip.c_str());
  return EXIT_SUCCESS;
}

// LRUCache lruCache(2);
// string url1 = "www.google.com";
// ResponseHead res1 = ResponseHead();
// lruCache.put(url1, res1);
// string url2 = "www.baidu.com";
// ResponseHead res2 = ResponseHead();
// lruCache.put(url2, res2);
// string url3 = "www.qq.com";
// cout << lruCache.inCache(url3) << endl;
// ResponseHead res3 = ResponseHead();
// lruCache.put(url3, res3);
// TimeStamp responseTime("Sun, 14 Feb 2022 21:24:00 GMT");
// time_t t = std::time(0);  // get time now
// tm* now = std::localtime(&t);
// now->tm_year += 1900;
// double diff = difftime(mktime(now), mktime(responseTime.get_t()));
// cout << diff << endl;