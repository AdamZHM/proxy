#include "httpheader.hpp"

// char *convertStringToChar(const std::string &s) {
//   char *m_array = (char *)malloc(s.length());
//   std::strcpy(m_array, s.c_str());
//   return m_array;
// }

HttpHeader::HttpHeader(const char *buffer) {
  std::string bufferStr(buffer);
  std::istringstream ss(buffer);
  std::string line;
  int i = 0;
  char typeOfRequest;  //'C' means connect, 'P' means post,'G' means get
  while (std::getline(ss, line)) {
    if (line.find("\r") == 0) {
      break;
    }
    if (i == 0) {  // first line
      this->first_line = line.substr(0,line.find("\r"));
      if (line.find("GET") == 0) {
        method = std::string("GET");
        int startIdx = line.find(' ') + 1;
        url = line.substr(startIdx, line.find(" HTTP") - startIdx);
        typeOfRequest = 'G';
      } else if (line.find("POST") == 0) {
        method = std::string("POST");
        int startIdx = line.find(' ') + 1;
        url = line.substr(startIdx, line.find(" HTTP") - startIdx);
        typeOfRequest = 'P';
      } else if (line.find("CONNECT") == 0) {
        method = std::string("CONNECT");
        typeOfRequest = 'C';
      } else {
        method = "";
      }
    } else {
      if (line.find("Host") == 0) {  // The line that starts with Host
        if (typeOfRequest != 'C') {
          int startIdx = line.find(":") + 2;
          if (line.find(":") != line.rfind(":")) {
            host = line.substr(startIdx, line.rfind(":") - startIdx);
            port = line.substr(line.rfind(":") + 1,
                               line.rfind('\r') - line.rfind(":") - 1);
          } else {
            port = "80";
            host = line.substr(startIdx, line.find("\r") - startIdx);
          }
        } else {  // for conncet
          int startIdx = line.find(":") + 2;
          host = line.substr(startIdx, line.rfind(":") - startIdx);
          port = line.substr(line.rfind(":") + 1,
                             line.rfind('\r') - line.rfind(":") - 1);
        }
      } else if (line.find("Cache-Control") == 0) {
        int startIdx = line.find(':') + 2;
        cache_control = line.substr(startIdx, line.find("\r") - startIdx);
        // std::cout << "--------------------------------------------------\n";
        // std::cout << "_______________" << cache_control << "____________";
        // std::cout << '\n';
      }
    }
    i++;
  }
}

//"GET www.bbc.co.uk/ HTTP/1.1" from 1.2.3.4 @ Sun Jan 1 22:58:17 2017
// char *HttpHeader::parseInLogFormat(const char *client_ip) {
//   std::stringstream logLine;
//   time_t t = time(NULL);
//   struct tm *tm = localtime(&t);
//   logLine << "\"" << method << " " << url << "\" from "
//           << std::string(client_ip) << " @ " << std::string(asctime(tm))
//           << "\n";
//   return convertStringToChar(logLine.str());
// }
