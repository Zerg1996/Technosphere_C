#include <iostream>
#include <algorithm>
#include <set>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 3100
#define MAXDATASIZE 1024

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
    static char buf[1024];
    int sockfd, numbytes;
    struct hostent *he;
    struct sockaddr_in their_addr; // адрес сервера
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }
    their_addr.sin_family = AF_INET;    // системный порядок
    their_addr.sin_port = htons(PORT);  // сетевой порядок
    memset(&(their_addr.sin_zero), '\0', 8);  // обнуляем остаток структуры
    if (argc > 1) {
        if (inet_aton(argv[1], &(their_addr.sin_addr)) == 0) {
            perror("bad");
            exit(1);
        }
    } else {
        char ip[16];
        sscanf("127.0.0.1","%s",ip);
        if (inet_aton(ip, &(their_addr.sin_addr)) == 0) {
            perror("bad");
            exit(1);
        }
    }
    if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
        perror("connect");
        exit(1);
    }
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, MSG_NOSIGNAL)) == -1) {
        perror("recv");
        exit(1);
    }
    buf[numbytes] = '\0';
    printf("Принял: %s",buf);
    set_nonblock(sockfd);
	set_nonblock(0);
    fd_set Set;
    FD_ZERO(&Set);
    FD_SET(sockfd, &Set);
    FD_SET(0,&Set);
    int fdmax = sockfd;
    for(;;) {
        select(fdmax+1, &Set, NULL, NULL, NULL);
        for(int i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &Set)) {
                if (i != 0) {
                    memset(buf, '\0', 1024);
                    int nbytes = recv(i, buf, sizeof(buf), 0);
                    if (nbytes > 0) {
                        cout << inet_ntoa(their_addr.sin_addr) << ':';
                        cout << buf << endl;
                        FD_SET(0, &Set);
                    } else {
                        if (!nbytes) {
                            cout << "Server was closed, so i close!" << endl;
                            close(sockfd);
                            return 0;
                        }
                    }
                } else {
                    fgets(buf,MAXDATASIZE - 1,stdin);
                    send(sockfd, buf,strlen(buf) - 1, 0);
                    FD_SET(sockfd, &Set);
                }
            }
        }
    }
	close(sockfd);
	return 0;
}
