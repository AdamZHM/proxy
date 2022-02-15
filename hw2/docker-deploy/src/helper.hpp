#ifndef __HELPER_HPP__
#define __HELPER_HPP__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <netdb.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <vector>
#include <algorithm>

int check_recv(int byte_count);
int check_send(int byte_count);
std::vector<char> convertCharStarToVecChar(const char * buffer, int size);
void changeHeaderToLower(std::string & line, std::string & temp);
string accept_connection(int socket_fd, int *client_connection_fd);

#endif