#include "httpheader.hpp"

char* convertStringToChar(const std::string& s) {
  char* m_array = (char *) malloc(s.length());
  std::strcpy(m_array, s.c_str());
  return m_array;
}

HttpHeader::HttpHeader(const char * buffer) {
  std::string bufferStr(buffer);
  std::istringstream ss(buffer);
  std::string line;
  int i = 0;
  while (std::getline(ss, line)) {
    if (i == 0) {
      in start = line.find(' ')+1;
      if(line.find("\r")!=string::nops){
        cout << "YES! WE FIND A r here\n"
        url = convertStringToChar(line.substr(start,line.find("\r") - start);
      }else{
        cout << "NO FINDING \n";
      }

    }
    if (line.find("GET") == 0) {  // first line
      method = convertStringToChar(std::string("GET"));
    } else if (line.find("POST") == 0) {
      method = convertStringToChar(std::string("POST"));
    } else if (line.find("CONNECT") == 0) {
      method = convertStringToChar(std::string("CONNECT"));
    } else if (line.find("Host") == 0) {
      int startIdx = line.find(":") + 2;
      host = convertStringToChar(line.substr(startIdx, line.find("\r") - startIdx));
    } else {
      break;
    }
  }
  i++;
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
