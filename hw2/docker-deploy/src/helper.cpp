#include "helper.hpp"

using namespace std;

bool daemonize() {
  int fd;
  int status;
  int pid;

  switch (fork()) {
    case -1:
      cout << "fork failed" << endl;
      return false;
    case 0:
      break;
    default:
      cout << "father process exit" << endl;
      exit(EXIT_SUCCESS);
  }

  status = setsid();
  if (status == -1) {
    cout << "setsid failed" << endl;
    exit(EXIT_FAILURE);
  }

  switch (fork()) {
    case -1:
      cout << "fork failed" << endl;
      return false;
    case 0:
      break;
    default:
      cout << "father process exit" << endl;
      exit(EXIT_SUCCESS);
  }
  umask(0);
  chdir("/");

  fd = open("/dev/null", O_RDWR);
  if (fd == -1){
    cout << "open(dev/null failed" << endl;
    exit(EXIT_FAILURE);
  }

  if (dup2(fd, STDIN_FILENO) == -1 || dup2(fd, STDOUT_FILENO) == -1 || dup2(fd, STDERR_FILENO) == 1){
    exit(EXIT_FAILURE);
  }

  if (fd > STDERR_FILENO && close(fd) == -1){
    exit(EXIT_FAILURE);
  }

  return true;
}

int check_recv(int byte_count) {
  if (byte_count < 0) {
    cerr << "errors happen when receiving data!" << endl;
    return -1;
  }
  if (byte_count == 0) {  // connection shut down
    cerr << "connection shut down" << endl;
    return 0;
  }
  return byte_count;
}

int check_send(int byte_count) {
  if (byte_count == -1) {
    cerr << "errors happen when sending data!" << endl;
    return -1;
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
  *client_connection_fd =
      accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
  if (*client_connection_fd == -1) {
    cerr << "Error: cannot accept connection on socket" << endl;
    return "";
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

int create_server(const char *port) {
  int status;
  int socket_fd;
  struct addrinfo host_info;        // hints
  struct addrinfo *host_info_list;  // results
  const char *hostname = "vcm-24353.vm.duke.edu";
  // const char *hostname = "vcm-24353.vm.duke.edu";
  // const char *port     = "12345";

  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags = AI_PASSIVE;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for host in server" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  }

  // use os randomly assigned port.
  if (port == NULL) {
    struct sockaddr_in *addr = (struct sockaddr_in *)(host_info_list->ai_addr);
    addr->sin_port = 0;
  }

  socket_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  }  // if

  int yes = 1;
  status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot bind socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  }  // if

  status = listen(socket_fd, 100);
  if (status == -1) {
    cerr << "Error: cannot listen on socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  }  // if

  // cout << "Waiting for connection on port " << port << endl;
  freeaddrinfo(host_info_list);
  return socket_fd;
}

//"GET www.bbc.co.uk/ HTTP/1.1" from 1.2.3.4 @ Sun Jan 1 22:58:17 2017
void printRequest(std::ofstream &fout, Client *client, std::string url) {
  std::stringstream logLine;
  time_t t = time(0);
  struct tm *tm = gmtime(&t);
  std::string time = std::string(asctime(tm));
  time.pop_back();
  fout << client->id << ": \"" << url << "\" from " << client->ip << " @ "
       << time << endl;
}

void printNotInCache(std::ofstream &fout, Client *client) {
  fout << client->id << ": not in cache" << endl;
}

void printContactServer(std::ofstream &fout, Client *client) {
  fout << client->id << ": Requesting \"" << client->first_line << "\" from "
       << client->url << endl;
}

void printInCacheValid(std::ofstream &fout, Client *client) {
  fout << client->id << ": in cache, valid" << endl;
}

void printInCacheReVal(std::ofstream &fout, Client *client) {
  fout << client->id << ": in cache, requires validation" << endl;
}

void printInCacheButExpired(std::ofstream &fout, Client *client, TimeStamp date,
                            std::string expires, int max_age) {
  if (expires != "") {
    TimeStamp expireTime(expires);
    time_t t = mktime(expireTime.get_t());
    struct tm *tm = gmtime(&t);
    std::string time = std::string(asctime(tm));
    time.pop_back();
    fout << client->id << ": in cache, but expired at " << time << endl;
  } else if (max_age > 0) {
    time_t t = mktime(date.get_t());
    t += max_age;
    struct tm *tm = gmtime(&t);
    std::string time = std::string(asctime(tm));
    time.pop_back();
    fout << client->id << ": in cache, but expired at " << time << endl;
  }
}

void printReceive(std::ofstream &fout, Client *client, std::string response) {
  fout << client->id << ": Received \"" << response << "\" from " << client->url
       << endl;
}

void printResponding(std::ofstream &fout, Client *client,
                     std::string response) {
  fout << client->id << ": Responding \"" << response << endl;
}

void printCloseTunnel(std::ofstream &fout, Client *client) {
  fout << client->id << ": Tunnel closed" << endl;
}