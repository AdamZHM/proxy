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
  // if (byte_count != sizeof(buffer)){
  //   cerr << "Error: send the broken message to the origin server" << endl;
  //   exit(EXIT_FAILURE);
  // }
  return byte_count;
}

std::vector<char> convertCharStarToVecChar(const char* buffer, int size) {
  std::vector<char> ans;
  for (int i = 0; i < size; i++) {
    ans.push_back(buffer[i]);
  }
  return ans;
}

void changeHeaderToLower(std::string& line, std::string& temp) {
  temp = line;
  std::transform(temp.begin(), temp.end(), temp.begin(),
                 [](unsigned char c) { return std::tolower(c); });
}