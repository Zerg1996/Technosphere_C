#include <iostream>
#include <algorithm>
#include <set>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/epoll.h>
#include <errno.h>
#include <string.h>
#include <string.h>
#include <arpa/inet.h>
#include <vector>
#define MAX_EVENTS 32
#define PORT 3100

using namespace std;

int set_nonblock(int fd)
{
	int flags;
	if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
		flags = 0;
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main(int argc, char **argv)
{
    int yes=1;
	int MasterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (setsockopt(MasterSocket,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }
	if(MasterSocket == -1) {
		std::cout << strerror(errno) << std::endl;
		return 1;
	}
	struct sockaddr_in SockAddr;
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = htons(PORT);
	SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(&(SockAddr.sin_zero), '\0', 8);
	int Result = bind(MasterSocket, (struct sockaddr *)&SockAddr, sizeof(SockAddr));
	if(Result == -1) {
		std::cout << strerror(errno) << std::endl;
		return 1;
	}
	set_nonblock(MasterSocket);
	Result = listen(MasterSocket, SOMAXCONN);
	if(Result == -1) {
		std::cout << strerror(errno) << std::endl;
		return 1;
	}
	struct epoll_event Event;
	Event.data.fd = MasterSocket;
	Event.events = EPOLLIN;
    vector <int> fd;
	struct epoll_event * Events;
	Events = (struct epoll_event *) calloc(MAX_EVENTS, sizeof(struct epoll_event));
	int EPoll = epoll_create1(0);
	epoll_ctl(EPoll, EPOLL_CTL_ADD, MasterSocket, &Event);
	while(true) {
		int N = epoll_wait(EPoll, Events, MAX_EVENTS, -1);
		for(unsigned int i = 0; i < N; i++) {
			if(Events[i].data.fd == MasterSocket) {
                cout << "accepted connection" << endl;
				int SlaveSocket = accept(MasterSocket, 0, 0);
				set_nonblock(SlaveSocket);
				struct epoll_event Event;
				Event.data.fd = SlaveSocket;
				send(Event.data.fd, "Welcome\n", 8, 0);
				Event.events = EPOLLIN | EPOLLET;
				epoll_ctl(EPoll, EPOLL_CTL_ADD, SlaveSocket, &Event);
				fd.push_back(Event.data.fd);
			} else {
				static char Buffer[1024];
				int RecvSize = recv(Events[i].data.fd, Buffer, 1024, MSG_NOSIGNAL);
				if (RecvSize == 0) {
                    cout << "connection terminated" << endl;
                    shutdown(Events[i].data.fd, SHUT_RDWR);
                    int erase_need = Events[i].data.fd;
                    auto j = fd.begin();
                    for (; *j != erase_need; ++j);
                    fd.erase(j);
                    close(Events[i].data.fd);
				}
				for (int j = 0; j < fd.size(); ++j) {
                    send(fd[j], Buffer, RecvSize, MSG_NOSIGNAL);
				}
			}
        }
	}
	return 0;
}
