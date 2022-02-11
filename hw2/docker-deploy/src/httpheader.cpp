#include "httpheader.hpp"

char * convertStringToChar(const std::string & s) {
  char * m_array = new char[s.length()];
  std::strcpy(m_array, s.c_str());
  return m_array;
}

HttpHeader::HttpHeader(const char * buffer) {
  std::string bufferStr(buffer);
  std::istringstream ss(buffer);
  std::string line;
  int i = 0;
  while (std::getline(ss, line)) {
    if (i == 0) {  //first line
      if (line[0] == 'G') {
        method = convertStringToChar(std::string("GET"));
      }
      else if (line[0] == 'P') {
        method = convertStringToChar(std::string("POST"));
      }
      else if (line[0] == 'C') {
        method = convertStringToChar(std::string("CONNECT"));
      }
      else {
        std::cerr << "Can't recogonize this HTTP method" << '\n';
        exit(EXIT_FAILURE);
      }
      url = convertStringToChar(line.substr(line.find(' ')+1));
    }
    else {
      int index = line.find("Host");
      if (index == 0) {  // The line that starts with Host
        std::cout << "Now this is a new line -----------\n";
        std::cout<<line<<'\n';
        host = convertStringToChar(line.substr(line.find(": ")+2));
        break;
      }
    }
    i++;
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
