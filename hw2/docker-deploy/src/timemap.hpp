#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <unordered_map>

class TimeMap {
 public:
  std::unordered_map<std::string, int> month;
  std::unordered_map<std::string, int> week;
  TimeMap() {
    month.insert(std::make_pair<std::string, int>("Jan", 0));
    month.insert(std::make_pair<std::string, int>("Feb", 1));
    month.insert(std::make_pair<std::string, int>("Mar", 2));
    month.insert(std::make_pair<std::string, int>("Apr", 3));
    month.insert(std::make_pair<std::string, int>("May", 4));
    month.insert(std::make_pair<std::string, int>("Jun", 5));
    month.insert(std::make_pair<std::string, int>("Jul", 6));
    month.insert(std::make_pair<std::string, int>("Aug", 7));
    month.insert(std::make_pair<std::string, int>("Sep", 8));
    month.insert(std::make_pair<std::string, int>("Oct", 9));
    month.insert(std::make_pair<std::string, int>("Nov", 10));
    month.insert(std::make_pair<std::string, int>("Dec", 11));
  }
};