#include "httpheader.hpp"

#include <string>
#include <iostream>
char* convertStringToChar(const std::string& s) {
  char* m_array = (char*)malloc(s.length());
  std::strcpy(m_array, s.c_str());
  return m_array;
}


HttpHeader::HttpHeader(const char * buffer) {
  std::string bufferStr(buffer);
  std::istringstream ss(buffer);
  std::string line;
  int i = 0;
  while (std::getline(ss, line)) {
    if (line.find("\r") == 0){
      break;
    }
    if (i == 0) {
      int startIdx = line.find(' ')+1;
      url = convertStringToChar(line.substr(startIdx, line.find('\r') - startIdx));
    }
    if (line.find("GET") == 0) {  // first line
      method = convertStringToChar(std::string("GET"));
    } else if (line.find("POST") == 0) {
      method = convertStringToChar(std::string("POST"));
    } else if (line.find("CONNECT") == 0) {
      method = convertStringToChar(std::string("CONNECT"));
    } else if (line.find("Host") == 0) {
      int startIdx = line.find(":") + 2;
      if (strcmp("CONNECT", method) == 0) {
        host = convertStringToChar(
            line.substr(startIdx, line.find(":443") - startIdx));
      } else {
        host = convertStringToChar(
            line.substr(startIdx, line.find("\r") - startIdx));
      }
      host = convertStringToChar(line.substr(startIdx, line.find("\r") - startIdx));
    } else if(line.find("Cache-Control") == 0){
      int startIdx = line.find(':') + 2;
      cache_control = convertStringToChar(line.substr(startIdx, line.find("\r") - startIdx));
      std::cout << "--------------------------------------------------\n";
      std::cout << cache_control<<'*';
      std::cout <<'\n';
    } 
  }
}


//"GET www.bbc.co.uk/ HTTP/1.1" from 1.2.3.4 @ Sun Jan 1 22:58:17 2017
char * HttpHeader::parseInLogFormat(const char * client_ip) {
  std::stringstream logLine;
  time_t t = time(NULL);
  struct tm * tm = localtime(&t);
  logLine << "\"" << method << " " << url << "\" from " << std::string(client_ip) << " @ "
          << std::string(asctime(tm)) << "\n";
  return convertStringToChar(logLine.str());
}
