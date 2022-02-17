#ifndef __HELPER_HPP__
#define __HELPER_HPP__

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "client.hpp"
#include "timestamp.cpp"

int check_recv(int byte_count);
int check_send(int byte_count);
std::vector<char> convertCharStarToVecChar(const char *buffer, int size);
void changeHeaderToLower(std::string &line, std::string &temp);
std::string accept_connection(int socket_fd, int *client_connection_fd);
int create_server(const char *port);
void printRequest(std::ofstream &fout, Client *client, std::string url);
void printNotInCache(std::ofstream &fout, Client *client);
void printContactServer(std::ofstream &fout, Client *client);
void printInCacheValid(std::ofstream &fout, Client *client);
void printInCacheReVal(std::ofstream &fout, Client *client);
void printInCacheButExpired(std::ofstream &fout, Client *client, TimeStamp date,
                            std::string expires, int max_age);
void printReceive(std::ofstream &fout, Client *client, std::string response);
void printCloseTunnel(std::ofstream &fout, Client *client);
void printResponding(std::ofstream &fout, Client *client, std::string response);

#endif