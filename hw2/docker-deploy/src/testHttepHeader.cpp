#include <iostream>

#include "httpheader.hpp"

int main() {
  std::string test("GET http://info.cern.ch/ HTTP/1.1\n""Host: info.cern.ch\n");
  HttpHeader hh(test.c_str());
  std::string ip = "11.1.1.1.1";
  std::cout << hh.parseInLogFormat(ip.c_str());
  return EXIT_SUCCESS;
}
