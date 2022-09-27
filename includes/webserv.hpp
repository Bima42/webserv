#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# define BUFFER_SIZE 4096

/***********************************************************************************
 *                              LIBRARIES                                          *
 ***********************************************************************************/
#include <map>
#include <stack>
#include <vector>
#include <iostream>
#include <string>

/***********************************************************************************
 *                              FUNCTIONS                                          *
 ***********************************************************************************/
//#include <sys/event.h> //kqueue and kevent. does not compile for some reasons

#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>

#endif // WEBSERV_HPP
