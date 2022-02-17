#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <unordered_map>

#include "timemap.cpp"

// int tm_sec;   // seconds of minutes from 0 to 61
//   int tm_min;   // minutes of hour from 0 to 59
//   int tm_hour;  // hours of day from 0 to 24
//   int tm_mday;  // day of month from 1 to 31
//   int tm_mon;   // month of year from 0 to 11
//   int tm_year;  // year since 1900
//   int tm_wday;  // days since sunday
//   int tm_yday;  // days since January 1st
class TimeStamp {
  struct tm t;

 public:
  TimeStamp(){}
  struct tm *get_t() {
    return &t;
  }
  explicit TimeStamp(const std::string &line) {
    TimeMap timemap;
    int offset = 0;
    // Date: Sun, 13 Feb 2022 15:51:34 GMT
    // if (line.find("Date") == 0) {
    //   offset = 6;
    // } else if (line.find("Expires") == 0) {
    //   offset = 9;
    // } else if (line.find("Last-Modified") == 0) {
    //   offset = 15;
    // }
    t.tm_sec = atoi(line.substr(23 + offset, 2).c_str());
    t.tm_min = atoi(line.substr(20 + offset, 2).c_str());
    t.tm_hour = atoi(line.substr(17 + offset, 2).c_str());
    t.tm_mday = atoi(line.substr(5 + offset, 2).c_str());
    std::unordered_map<std::string, int>::const_iterator month =
        timemap.month.find(line.substr(8 + offset, 3));
    t.tm_mon = month->second;
    t.tm_year = atoi(line.substr(12 + offset, 4).c_str());
    t.tm_isdst = 0;
  }
  double timediff(TimeStamp timestamp) {
    //   std::string s1 = "Date: Sun, 13 Feb 2022 15:51:34 GMT";
    // std::string s2 = "Date: Sun, 13 Feb 2022 15:59:34 GMT";
    return difftime(mktime(this->get_t()), mktime(timestamp.get_t()));
  }
};