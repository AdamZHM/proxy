#include "helper.hpp"

using namespace std;

int check_recv(int byte_count) {
  if (byte_count < 0) {
    cerr << "errors happen when receiving data!" << endl;
    exit(EXIT_FAILURE);
  }
  if (byte_count == 0) {  // connection shut down
    cerr << "connection shut down" << endl;
    exit(EXIT_FAILURE);
  }
  return byte_count;
}

int check_send(int byte_count) {
  if (byte_count == -1) {
    cerr << "errors happen when sending data!" << endl;
    exit(EXIT_FAILURE);
  }
  return byte_count;
}

std::vector<char> convertCharStarToVecChar(const char *buffer, int size) {
  std::vector<char> ans;
  for (int i = 0; i < size; i++) {
    ans.push_back(buffer[i]);
  }
  return ans;
}

void changeHeaderToLower(std::string &line, std::string &temp) {
  temp = line;
  std::transform(temp.begin(), temp.end(), temp.begin(),
                 [](unsigned char c) { return std::tolower(c); });
}

string accept_connection(int socket_fd, int *client_connection_fd) {
  struct sockaddr_in *socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  *client_connection_fd = accept(
      socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
  if (*client_connection_fd == -1) {
    cerr << "Error: cannot accept connection on socket" << endl;
    exit(EXIT_FAILURE);
  }  // if
  struct sockaddr_in *pV4Addr = (struct sockaddr_in *)&socket_addr;
  struct in_addr ipAddr = pV4Addr->sin_addr;
  char *ip = (char *)malloc(INET_ADDRSTRLEN);
  inet_ntop(AF_INET, &ipAddr, ip, INET_ADDRSTRLEN);
  // this->set_client_connection_fd(*client_connection_fd);
  string ans(ip);
  free(ip);
  return ans;
}