#include "server.hpp"
using namespace std;

LRUCache lruCache(100);
ofstream fout("./proxy.log");
mutex mtx;

string Server::accept_connection(int socket_fd, int *client_connection_fd) {
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
  this->set_client_connection_fd(*client_connection_fd);
  string ans(ip);
  free(ip);
  return ans;
}

void Server::deal_with_get_request(Client &proxy_as_client, const char *url,
                                   char *buffer, Client *client) {
  int buffer_size = 100000;
  char response[buffer_size];
  memset(response, 0, buffer_size);
  string str_buffer(buffer);
  cout << buffer << endl;
  if (send(proxy_as_client.get_socket_fd(), buffer,
           str_buffer.find("\r\n\r\n") + 4, 0) < 0) {
    cerr << "Error : send data to server in GET" << endl;
    return;
  }
  int bytes_recv =
      recv(proxy_as_client.get_socket_fd(), response, buffer_size, 0);
  if (bytes_recv == 0) {
    cout << "Server shut down the connection" << endl;
    proxy_as_client.close_socket_fd();
    client->close_socket_fd();
    return;
  } else if (bytes_recv < 0) {
    cerr << "Error: recv data from server" << endl;
    return;
  }
  ResponseHead rph;
  rph.initResponse(response, bytes_recv);
  mtx.lock();
  printContactServer(fout, client);
  mtx.unlock();
  if (rph.if_chunked) {
    cout << "__________________data is chunked______________" << endl;
    // if data is chunked

    send(client->get_socket_fd(), response, bytes_recv, 0);
    while (true) {
      memset(response, 0, buffer_size);
      int len_recv =
          recv(proxy_as_client.get_socket_fd(), response, buffer_size, 0);
      // cout << response << endl;
      if (len_recv <= 0) {
        cout << "no more chunked message" << endl;
        break;
      } else {
        send(client->get_socket_fd(), response, len_recv, 0);
      }
    }
    cout << "___________chunked data over_________________" << endl;
  } else {
    int responseLength = rph.totalLength();
    string str_url(url);
    // TODO handle not chunked data
    // if no store, we do not put it into cache
    while (rph.response.size() < responseLength) {
      int len_recv =
          recv(proxy_as_client.get_socket_fd(), response, sizeof(response), 0);
      if (len_recv == 0) {
        cerr << "502" << endl;
        return;
      } else if (len_recv == -1) {
        perror("recv error in get");
        return;
      } else {
        rph.appendResponse(response, len_recv);
      }
    }
    rph.printCacheReponseToGetReq(fout, client);
    send(client->get_socket_fd(), rph.response.data(), rph.response.size(), 0);

    proxy_as_client.close_socket_fd();
    client->close_socket_fd();

    cout << "__________save " << url << " in to cache_______________" << endl;
    // TODO log cache situation
    if (!rph.if_no_store) {
      mtx.lock();
      lruCache.put(str_url, rph);
      mtx.unlock();
    }
  }
  mtx.lock();
  printReceive(fout, client, rph.status);
  printResponding(fout, client, rph.status);
  mtx.unlock();
  // cout << lruCache.get(str_url).response.data() << endl;
  proxy_as_client.close_socket_fd();
  client->close_socket_fd();
}

void Server::deal_with_connect_request(Client &proxy_as_client,
                                       Client *client) {
  const char *msg = "HTTP/1.1 200 OK\r\n\r\n";
  if (send(client->get_socket_fd(), msg, 19, 0) < 0) {
    cerr << "Error send data in connect " << endl;
    return;
  }
  int buffer_size = 100000;
  vector<int> fd_vector = {proxy_as_client.get_socket_fd(),
                           client->get_socket_fd()};
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
      return;
    }
    for (int i = 0; i < 2; i++) {
      char buffer[buffer_size];
      memset(buffer, 0, buffer_size);
      if (FD_ISSET(fd_vector[i], &readfds)) {
        int bytes_recv = recv(fd_vector[i], buffer, buffer_size, 0);
        if (bytes_recv == 0) {
          client->close_socket_fd();
          proxy_as_client.close_socket_fd();
          return;
        } else if (bytes_recv < 0) {
          cerr << "Error : recv data error" << endl;
          return;
        }
        buffer[bytes_recv] = '\0';

        if (send(fd_vector[(i + 1) % 2], buffer, bytes_recv, 0) < 0) {
          cerr << "Error : send data error" << endl;
          return;
        }
      }
    }
  }
}

void Server::deal_with_post_request(Client &proxy_as_client, const char *url,
                                    int content_length, char *buffer,
                                    Client *client) {
  int buffer_size = 100000;
  string str_buffer(buffer);
  if (send(proxy_as_client.get_socket_fd(), buffer, buffer_size, 0) < 0) {
    cerr << "Error : send data to server in POST" << endl;
    return;
  }
  char response[buffer_size];
  memset(response, 0, buffer_size);
  int bytes_recv =
      recv(proxy_as_client.get_socket_fd(), response, buffer_size, 0);
  if (bytes_recv == 0) {
    cout << "Server shut down the connection" << endl;
    proxy_as_client.close_socket_fd();
    client->close_socket_fd();
    // this->close_client_connection_fd();
    return;
  } else if (bytes_recv < 0) {
    cerr << "Error: recv data from server" << endl;
    return;
  }
  response[bytes_recv] = '\0';
  ResponseHead rph;
  rph.initResponse(response, bytes_recv);
  // cout << "content-length: " << rph.content_length << endl;
  mtx.lock();
  printContactServer(fout, client);
  mtx.unlock();
  string str_url(url);
  // TODO check \r\n\r\n in response, if not, 502
  while (true) {
    memset(response, 0, 100000);
    int len_recv =
        recv(proxy_as_client.get_socket_fd(), response, sizeof(response), 0);
    if (len_recv <= 0) {
      cout << "no more data" << endl;
      break;
    } else {
      rph.appendResponse(response, len_recv);
    }
  }
  send(client->get_socket_fd(), rph.response.data(), rph.response.size(), 0);
  mtx.lock();
  printReceive(fout, client, rph.status);
  printResponding(fout, client, rph.status);
  mtx.unlock();
  proxy_as_client.close_socket_fd();
  client->close_socket_fd();
}

void Server::handle_request(Client *client) {
  int buffer_size = 100000;
  char buffer[buffer_size];
  memset(buffer, 0, sizeof(buffer));
  int byte_recv = recv(client->get_socket_fd(), buffer, buffer_size, 0);
  if (byte_recv == -1) {
    client->close_socket_fd();
    std::cout << "_________RETURN_______\n" << std::endl;
    return;
  }
  buffer[byte_recv] = '\0';
  std::cout << "_________THIS IS USER REQUEST_______\n" << buffer << std::endl;
  HttpHeader httpHeader(buffer);
  const char *host = httpHeader.get_host();

  const char *method = httpHeader.get_method();
  const char *port = httpHeader.get_port();
  const char *first_line = httpHeader.get_first_line();
  if (strcmp("", method) == 0) {
    std::cout << "_________HTTP/1.1 400 Bad Request__________\n"
              << buffer << std::endl;
    return;
  }
  string url(httpHeader.get_url());
  // create proxy as client here
  Client proxy_as_client;
  int client_fd = proxy_as_client.createClient(host, port);
  if (client_fd == -1) {
    std::cout << "_________HTTP 400 Bad Request__________\n"
              << buffer << std::endl;
    return;
  }
  string str_host(host);
  proxy_as_client.host = str_host;
  string str_first_line(first_line);
  proxy_as_client.first_line = str_first_line;
  proxy_as_client.url = url;

  client->first_line = httpHeader.get_first_line();
  client->url = url;

  if (strcmp(method, "GET") == 0) {
    std::cout << "_________THIS IS GET______\n" << std::endl;
    // url in cache
    mtx.lock();
    bool ifInCache = lruCache.inCache(url);  // check if in cache
    printRequest(fout, client, httpHeader.get_first_line());
    mtx.unlock();
    if (ifInCache) {
      bool revalidate = true;  // true means revalidate succefully
      bool ifExpire = true;    // true means not expire
      mtx.lock();
      ResponseHead resp = lruCache.get(url);
      mtx.unlock();

      if (resp.if_cache_control == false || resp.if_no_cache == true ||
          resp.if_must_revalidate == true || resp.max_age == 0) {
        // TODO do revalidation, update expiration
        revalidate = this->revalidation(resp, proxy_as_client, client);
      } else if (resp.max_age != -1 || resp.expires != "") {
        ifExpire = this->ifExpired(resp, proxy_as_client, client);
      }
      if (revalidate && ifExpire) {
        // use cache
        mtx.lock();
        printInCacheValid(fout, client);
        printResponding(fout, client, resp.status);
        mtx.unlock();
        cout << "_____________use cache______________" << endl;
        send(client->get_socket_fd(), resp.response.data(),
             resp.response.size(), 0);
      } else {
        mtx.lock();
        if (!revalidate) {
          printInCacheReVal(fout, client);
        } else {
          printInCacheButExpired(fout, client, resp.date, resp.expires,
                                 resp.max_age);
        }
        mtx.unlock();
        cout << "_________cannot use cache___________" << endl;
        this->deal_with_get_request(proxy_as_client, httpHeader.get_url(),
                                    buffer, client);
        // if need revalidate, will do send in revalidation
      }
      // mtx.lock();
      // printResponding(fout, client, resp.status);
      // mtx.lock();
    } else {  // not in cache
      cout << "_____________not in cache______________" << endl;
      this->deal_with_get_request(proxy_as_client, httpHeader.get_url(), buffer,
                                  client);
      mtx.lock();
      printNotInCache(fout, client);
      mtx.unlock();
    }
  } else if (strcmp(method, "POST") == 0) {
    mtx.lock();
    printRequest(fout, client, httpHeader.get_first_line());
    mtx.unlock();
    this->deal_with_post_request(proxy_as_client, httpHeader.get_url(),
                                 httpHeader.get_content_length(), buffer,
                                 client);
  } else if (strcmp(method, "CONNECT") == 0) {  //
    std::cout << "_________THIS IS CONNECT______\n" << std::endl;
    this->deal_with_connect_request(proxy_as_client, client);
    mtx.lock();
    printCloseTunnel(fout, client);
    mtx.unlock();
  } else {
    std::cout << "_________THIS IS NOTHING______\n" << std::endl;
  }
  return;
}

bool Server::revalidation(ResponseHead &resp, Client &proxy_as_client,
                          Client *client) {
  if (resp.etag == "" && resp.last_modified == "" && resp.max_age == -1) {
    return true;
  }
  string send_to_server = "";
  send_to_server.append(proxy_as_client.first_line + "\r\n");
  send_to_server.append("Host: " + proxy_as_client.host + "\r\n");

  if (resp.etag != "") {
    send_to_server += "If-None-Match: " + resp.etag + "\r\n";
  }
  if (resp.last_modified != "") {
    send_to_server += "IF-Modified-Since: " + resp.last_modified + "\r\n";
  }
  send_to_server += "\r\n";
  cout << send_to_server << endl;
  char response[100000];
  memset(response, 0, 100000);
  send(proxy_as_client.get_socket_fd(), send_to_server.c_str(),
       send_to_server.length(), 0);
  int bytes_recv =
      recv(proxy_as_client.get_socket_fd(), response, sizeof(response), 0);
  ResponseHead temp;
  temp.initResponse(response, bytes_recv);
  cout << "________revalidation response: \n" << response << endl;
  resp.date = temp.date;  // update the resp.date
  mtx.lock();
  lruCache.put(proxy_as_client.url, resp);
  mtx.unlock();
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

bool Server::ifExpired(ResponseHead &resp, Client &proxy_as_client,
                       Client *client) {
  time_t t = std::time(0);  // get time now
  tm *now = gmtime(&t);
  // tm *now = std::localtime(&t);
  now->tm_year += 1900;
  bool valid = false;
  if (resp.max_age > 0) {
    TimeStamp responseTime = resp.date;
    double diff = difftime(mktime(now), mktime(responseTime.get_t()));
    // cout << diff << endl;
    if (diff <= resp.max_age) {
      cout << "___________data fresh, no expired by max-age_______" << endl;
      valid = true;
    } else {
      cout << "___________data not fresh, expired by max-age_______" << endl;
    }
  } else if (resp.expires != "") {
    TimeStamp expireTime(resp.expires);
    double diff = difftime(mktime(expireTime.get_t()), mktime(now));
    if (diff > 0) {
      cout << "___________data fresh, no expire by expireTime______" << endl;
      valid = true;
    } else {
      cout << "___________data not fresh, expired by expireTime______" << endl;
    }
  }
  if (valid == false) {
    valid = this->revalidation(resp, proxy_as_client, client);
  }
  return valid;
}