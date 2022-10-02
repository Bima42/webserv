#include "Looper.hpp"

/**************************************************************************************/
/*                          CONSTRUCTORS / DESTRUCTORS                                */
/**************************************************************************************/
Looper::Looper() : _config(), _max_fd(), _servers() {}

Looper::Looper(const Looper &other) : _config(other._config), _max_fd(other._max_fd), _servers(other._servers)
{
    *this = other;
}

Looper::~Looper() {}

/**************************************************************************************/
/*                                  MEMBER FUNCTIONS                                  */
/**************************************************************************************/

int Looper::setupLoop()
{
    long		fd;

    // this is for one single listen. we will see when parsing is done for more
    FD_ZERO(&_active_fd_set);
    _max_fd = 0;

    for (unsigned int i = 0; i < _servers.size(); i++)
    {
        if (_servers[i].buildServer() != -1)
        {
            fd = _servers[i].getFd();
            FD_SET(fd, &_active_fd_set);
            if (fd > _max_fd)
                _max_fd = fd;
        }
    }
    if (_max_fd == 0)
    {
        std::cerr << "Could not setup cluster !" << std::endl;
        return (-1);
    }
    else
        return (0);
}

void Looper::log(std::string message)
{
    std::cout << message << std::endl;
}

void Looper::addServer(Server &server)
{
    this->_servers.push_back(server);
}

void Looper::setMaxFd()
{
    _max_fd = 0;
    for (unsigned int i = 0; i < _servers.size(); i++)
    {
        if (_servers[i].getFd() > _max_fd)
            _max_fd = _servers[i].getFd();
    }
}

void	*ft_memcpy(void *dst, const void *src, size_t n)
{
    size_t			i;
    unsigned char	*p;
    unsigned char	*q;

    i = 0;
    p = (unsigned char *)dst;
    q = (unsigned char *)src;
    while (i < n)
    {
        p[i] = q[i];
        i++;
    }
    return (dst);
}

void Looper::loop()
{
    while (1)
    {
        fd_set		    reading_fd_set;
        fd_set		    writing_fd_set;
        struct timeval  timeout;
        int				ret = 0;

        while (ret == 0)
        {
            // Setting the timeout for select
            timeout.tv_sec  = 3;
            timeout.tv_usec = 0;
            // Copying the content for the reading set into the active set
            ft_memcpy(&reading_fd_set, &_active_fd_set, sizeof(_active_fd_set));
            FD_ZERO(&writing_fd_set);
            // here we set the already active fd's in the writing fd's
            for (std::vector<int>::iterator it = _ready_fd.begin(); it != _ready_fd.end(); it++)
                FD_SET(*it, &writing_fd_set);
            // select will wait for an event on the set given
            ret = select(_max_fd + 1, &reading_fd_set, &writing_fd_set, NULL, &timeout);
        }

        // ret will be greater than 0 if any valid event is catched
        if (ret > 0)
        {
            /*for (std::vector<int>::iterator it = _ready_fd.begin(); it != _ready_fd.end(); it++)
            {
                if (FD_ISSET(*it, &writing_fd_set))
                {
                    long ret_val = _active_servers[*it]->send(*it);

                    if (ret_val == 0)
                    {
                        _ready_fd.erase(it); // erase the fd from vector when comm is over
                    }
                    else if
                    {
                        // Here we will remove the fd we catched in the error
                        // and clear all communication and all open channels.
                        // The fd and the active_server will be removed too
                        FD_CLR(*it, &_active_fd_set);
                        FD_CLR(*it, &reading_fd_set);
                        _active_servers.erase(*it);
                        _ready_fd.erase(it);
                    }
                    ret = 0;
                    break;
                }
            }*/

            for (std::map<long, Server *>::iterator it = _active_servers.begin(); it != _active_servers.end(); it++)
            {
                long socket = it->first;

                if (FD_ISSET(socket, &reading_fd_set))
                {
                    long ret_val = it->second->readFromClient(socket); // TODO: place bima's code here

                    if (ret_val == 0)
                    {
                        // if there is nothing more to read, it's time to process the request
                       //it->second->process(socket, _config); // TODO: understand this part better
                       // we store the socket fd into our ready_fd vector since we want to keep the channel open
                       _ready_fd.push_back(socket);
                    }
                    else if (ret_val == -1)
                    {
                        // in case of error, we clear as done previously
                        FD_CLR(socket, &_active_fd_set);
                        FD_CLR(socket, &reading_fd_set);
                        _active_servers.erase(socket);
                        it = _active_servers.begin();
                    }
                    ret = 0;
                    break;
                }
            }
            for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); it++)
            {
                long fd = (*it).getFd();

                if (FD_ISSET(fd, &reading_fd_set))
                {
                    // Creating a socket with accept
                    long socket = (*it).createSocket();

                    // We check if the socket has been created
                    if (socket != -1)
                    {
                        // if it has a valid socket fd, we enter it into our active_fd_set
                        // and we add it into the _active_servers map with his fd as a key for the sock
                        FD_SET(socket, &_active_fd_set);
                        _active_servers.insert(std::make_pair<long int, Server *>((*it).getFd(), &(*it)));
                        for (std::map<long int, Server *>::iterator it = _active_servers.begin(); it != _active_servers.end(); ++it)
                            std::cout << (*it).first << std::endl;
                        // Setting max_fd if the new fd from the socket is greater
                        if (socket > _max_fd)
                            _max_fd = socket;
                    }
                    ret = 0;
                    break;
                }
            }
        }
        else
        {
            // An issue with select has been caught
            // In this case we will close every opened sockets, clear the active_servers map and the read_fd's
            // FD_ZERO to clear the active_fd_set and re set all the sockets to restart the server
            std::cout << "Select had an issue !" << std::cout;
            for (std::map<long, Server *>::iterator it = _active_servers.begin(); it != _active_servers.end(); it++)
                it->second->close(it->first);
            _active_servers.clear();
            _ready_fd.clear();
            FD_ZERO(&_active_fd_set);
            for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); it++)
                FD_SET((*it).getFd(), &_active_fd_set);
        }
    }
}

// TODO : Bind ??
// TODO : Ask Bima how the requests are stocked. Maybe we would use a map of <long, std::string> to hold the socket