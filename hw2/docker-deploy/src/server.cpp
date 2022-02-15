#include "server.hpp"

using namespace std;

LRUCache lruCache(10);

int Server::create_server(const char *port) {
  int status;
  int socket_fd;
  struct addrinfo host_info;        // hints
  struct addrinfo *host_info_list;  // results
  const char *hostname = "vcm-24373.vm.duke.edu";
  // const char *hostname = "vcm-24353.vm.duke.edu";
  // const char *port     = "12345";

  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags = AI_PASSIVE;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
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
  this->socket_fd = socket_fd;
  return socket_fd;
}

string Server::accept_connection() {
  struct sockaddr_in *socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  int client_connection_fd = accept(
      this->socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
  if (client_connection_fd == -1) {
    cerr << "Error: cannot accept connection on socket" << endl;
    exit(EXIT_FAILURE);
  }  // if
  struct sockaddr_in *pV4Addr = (struct sockaddr_in *)&socket_addr;
  struct in_addr ipAddr = pV4Addr->sin_addr;
  char *ip = (char *)malloc(INET_ADDRSTRLEN);
  inet_ntop(AF_INET, &ipAddr, ip, INET_ADDRSTRLEN);
  this->set_client_connection_fd(client_connection_fd);
  string ans(ip);
  free(ip);
  return ans;
}

void Server::close_client_connection_fd() { close(this->client_connection_fd); }

void Server::deal_with_get_request(Client proxy_as_client, const char *url,
                                   char *buffer) {
  // Client proxy_as_client;
  // proxy_as_client.createClient(host, port);
  int buffer_size = 65536;
  char response[buffer_size];
  memset(response, 0, buffer_size);
  cout << buffer << endl;
  if (send(proxy_as_client.get_socket_fd(), buffer, buffer_size, 0) < 0) {
    cerr << "Error : send data to server in GET" << endl;
    exit(EXIT_FAILURE);
  }
  int bytes_recv =
      recv(proxy_as_client.get_socket_fd(), response, buffer_size, 0);
  if (bytes_recv == 0) {
    cout << "Server shut down the connection" << endl;
    proxy_as_client.close_socket_fd();
    this->close_client_connection_fd();
    return;
  } else if (bytes_recv < 0) {
    cerr << "Error: recv data from server" << endl;
    exit(EXIT_FAILURE);
  }
  response[bytes_recv] = '\0';
  ResponseHead rph;
  rph.initResponse(response, bytes_recv);
  // cout << "content-length: " << rph.content_length << endl;
  if (rph.if_chunked) {
    cout << "__________________data is chunked______________" << endl;
    // if data is chunked

    send(this->get_client_connection_fd(), response, bytes_recv, 0);
    while (true) {
      memset(response, 0, buffer_size);
      int len_recv =
          recv(proxy_as_client.get_socket_fd(), response, buffer_size, 0);
      // cout << response << endl;
      if (len_recv <= 0) {
        cout << "no more chunked message" << endl;
        break;
      } else {
        response[len_recv] = '\0';
        send(this->get_client_connection_fd(), response, len_recv, 0);
      }
    }
    cout << "___________chunked data over_________________" << endl;
  } else {
    string str_url(url);
    // TODO handle not chunked data
    // if no store, we do not put it into cache
    if (rph.if_no_store) {
      send(this->get_client_connection_fd(), response, bytes_recv, 0);
    } else {  // then anything could be put into cache
      // first check length larger than buffer size?
      int content_length = rph.content_length;
      if (content_length > bytes_recv) {
        while (1) {
          memset(response, 0, buffer_size);
          bytes_recv =
              recv(proxy_as_client.get_socket_fd(), response, buffer_size, 0);
          if (bytes_recv <= 0) {
            cout << "recv long message end" << endl;
            break;
          }
          rph.appendResponse(response, bytes_recv);
        }
        send(this->get_client_connection_fd(), rph.response.data(),
             rph.response.size(), 0);
      } else {
        send(this->get_client_connection_fd(), response, bytes_recv, 0);
      }
      cout << "__________save in to cache_______________" << endl;
      // TODO log cache situation
      lruCache.put(str_url, rph);
    }
    cout << lruCache.get(str_url).response.data() << endl;
    proxy_as_client.close_socket_fd();
    this->close_client_connection_fd();
  }
}

void Server::deal_with_connect_request(Client proxy_as_client) {
  const char *msg = "HTTP/1.1 200 OK\r\n\r\n";
  if (send(this->get_client_connection_fd(), msg, 19, 0) < 0) {
    cerr << "Error send data in connect " << endl;
    exit(EXIT_FAILURE);
  }
  int buffer_size = 65536;
  vector<int> fd_vector = {proxy_as_client.get_socket_fd(),
                           this->get_client_connection_fd()};
  fd_set readfds;
  int max_fd =
      fd_vector[0] > fd_vector[1] ? fd_vector[0] + 1 : fd_vector[1] + 1;
  while (1) {
    FD_ZERO(&readfds);
    for (int i = 0; i < 2; i++) {
      FD_SET(fd_vector[i], &readfds);
    }
    int status = select(max_fd, &readfds, NULL, NULL, NULL);
    if (status == -1) {
      cerr << "Error in select" << endl;
      exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 2; i++) {
      char buffer[buffer_size];
      memset(buffer, 0, buffer_size);
      if (FD_ISSET(fd_vector[i], &readfds)) {
        int bytes_recv = recv(fd_vector[i], buffer, buffer_size, 0);
        if (bytes_recv == 0) {
          this->close_client_connection_fd();
          proxy_as_client.close_socket_fd();
          return;
        } else if (bytes_recv < 0) {
          cerr << "Error : recv data error" << endl;
          exit(EXIT_FAILURE);
        }
        buffer[bytes_recv] = '\0';

        if (send(fd_vector[(i + 1) % 2], buffer, bytes_recv, 0) < 0) {
          cerr << "Error : send data error" << endl;
          exit(EXIT_FAILURE);
        }
      }
    }
  }
}

void Server::handle_request() {
  string client_ip;
  int buffer_size = 65536;
  char buffer[buffer_size];
  memset(buffer, 0, sizeof(buffer));
  client_ip = this->accept_connection();
  int byte_recv =
      recv(this->get_client_connection_fd(), buffer, buffer_size, 0);
  if (byte_recv == -1) {
    this->close_client_connection_fd();
    std::cout << "_________RETURN_______\n" << std::endl;
    return;
  }
  buffer[byte_recv] = '\0';
  std::cout << "_________THIS IS USER REQUEST_______\n" << buffer << std::endl;
  HttpHeader httpHeader(buffer);
  const char *host = httpHeader.get_host();

  std::cout<<"_______________checking if host name correct here\n"<<host<<'\n';
  const char *method = httpHeader.get_method();
  const char *port = httpHeader.get_port();
  const char *first_line = httpHeader.get_first_line();
  string url(httpHeader.get_url());
  // create proxy as client here
  Client proxy_as_client;
  proxy_as_client.createClient(host, port);
  string str_host(host);
  proxy_as_client.host = str_host;
  string str_first_line(first_line);
  proxy_as_client.first_line = str_first_line;

  if (strcmp(method, "GET") == 0) {
    std::cout << "_________THIS IS GET______\n" << std::endl;
    // url in cache
    if (lruCache.inCache(url)) {
      bool canUseCache = false;
      ResponseHead resp = lruCache.get(url);
      //TODO: THis harcode is only for test,delete it later
      resp.if_cache_control =false;
      if (resp.if_cache_control == false) {
        // canUseCache = false;
        cout<< "________________check if firstLine here_____"<<proxy_as_client.first_line<<'\n';
        canUseCache = this->revalidation(resp, proxy_as_client);
      } else if (resp.if_no_cache == true || resp.if_must_revalidate == true) {
        // TODO do revalidation, update expiration
      } else if (resp.max_age != -1 || resp.expires != "") {
        // TODO do check expiration
      }
      if (canUseCache) {
        // use cache
        cout << "_____________use cache______________" << endl;
        send(this->client_connection_fd, resp.response.data(),
             resp.response.size(), 0);
      } else {
        // handle get and check if need save to cache
        this->deal_with_get_request(proxy_as_client, httpHeader.get_url(),
                                    buffer);
      }
    } else {  // not in cache
      this->deal_with_get_request(proxy_as_client, httpHeader.get_url(),
                                  buffer);
    }

  } else if (strcmp(method, "POST") == 0) {
    this->deal_with_post_request(proxy_as_client, httpHeader.get_url(), buffer);
  } else if (strcmp(method, "CONNECT") == 0) {  //
    std::cout << "_________THIS IS CONNECT______\n" << std::endl;
    this->deal_with_connect_request(proxy_as_client);
  } else {
    std::cout << "_________THIS IS NOTHING______\n" << std::endl;
  }
}

// bool Server::ifExpired() {}

bool Server::revalidation(ResponseHead resp, Client proxy_as_client) {
  if (resp.etag == "" && resp.last_modified == "") {
    return true;
  }
  cout << "+++++++++" << proxy_as_client.first_line << endl;
  string send_to_server = "";
  send_to_server.append("!!" + proxy_as_client.first_line);
  send_to_server.append("Host: " + proxy_as_client.host + "\r\n");

  // resp.etag = "*";
  if (resp.etag != "") {
    send_to_server += "If-None-Match: " + resp.etag + "\r\n";
  }
  if (resp.last_modified != "") {
    send_to_server += "IF-Modified-Since: " + resp.last_modified + "\r\n";
  }
  send_to_server += "\r\n";
  cout << send_to_server << endl;
  char response[65536];
  memset(response, 0, 65536);
  send(proxy_as_client.get_socket_fd(), send_to_server.c_str(), send_to_server.length(), 0);
  int bytes_recv =
      recv(proxy_as_client.get_socket_fd(), response, sizeof(response), 0);
  ResponseHead temp;
  temp.initResponse(response, bytes_recv);
  cout << "________revalidation response: \n" << response << endl;
  if (temp.status.find("304") != string::npos) {
    cout << "____________check etag or last_modified and data "
            "fresh_______________"
         << endl;
    return true;
  }
  cout << "____________check etag or last_modified and data not "
          "fresh___________"
       << endl;
  return false;
}