#if !defined(__POSIX_SOCKET_TEMPLATE_H__)
#define __POSIX_SOCKET_TEMPLATE_H__

#include <stdio.h>
#include <sys/types.h>
#if !defined(WIN32)
#include <sys/socket.h>
#include <netdb.h>
#include <sys/ioctl.h>
#if !defined(__APPLE__)
#include <linux/if.h>
#endif
#else
#include <ws2tcpip.h>
#endif
#if defined(__VMS)
#include <ioctl.h>
#endif
#include <fcntl.h>

/*
    A template for opening a non-blocking POSIX socket.
*/
int open_nb_socket(const char* addr, const char* port);

int open_nb_socket(const char* addr, const char* port, char *MACaddress) {
    struct addrinfo hints = {0};

    hints.ai_family = AF_UNSPEC; /* IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Must be TCP */
    int sockfd = -1;
    int rv;
    struct addrinfo *p, *servinfo;

    /* get address information */
    rv = getaddrinfo(addr, port, &hints, &servinfo);
    if(rv != 0) {
        fprintf(stderr, "Failed to open socket (getaddrinfo): %s\r\n", gai_strerror(rv));
        return -1;
    }

    /* open the first possible socket */
    for(p = servinfo; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) continue;

        /* connect to server */
        rv = connect(sockfd, p->ai_addr, p->ai_addrlen);
        if(rv == -1) {
          close(sockfd);
          sockfd = -1;
          continue;
        }

        #if !defined(WIN32) && !defined(__APPLE__)
        // get MAC address
        struct ifreq s;
        ioctl(sockfd, SIOCGIFHWADDR, &s);
        char buffer[6*3];
        int i;
        for (i = 0; i < 6; ++i) {
          sprintf(&buffer[i*3], "%02x:", (unsigned char) s.ifr_addr.sa_data[i]);
          //printf(" %02x", (unsigned char) s.ifr_addr.sa_data[i]);
        }
        //printf("\r\n");
        buffer[17] = '\0';

        std::string strMACaddress = std::string(buffer, 18);
        strMACaddress.copy(MACaddress, 18);
        // printf("  MAC address from posix_sockets %s\r\n", strMACaddress.c_str());
        #endif

        break;
    }  

    /* free servinfo */
    freeaddrinfo(servinfo);

    /* make non-blocking */
#if !defined(WIN32)
    if (sockfd != -1) fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL) | O_NONBLOCK);
#else
    if (sockfd != INVALID_SOCKET) {
        // original from MQTT-C
        // int iMode = 1;
        // changed for usage in OMOTE
        u_long iMode = 1;
        ioctlsocket(sockfd, FIONBIO, &iMode);
    }
#endif
#if defined(__VMS)
    /* 
        OpenVMS only partially implements fcntl. It works on file descriptors
        but silently fails on socket descriptors. So we need to fall back on
        to the older ioctl system to set non-blocking IO
    */
    int on = 1;                 
    if (sockfd != -1) ioctl(sockfd, FIONBIO, &on);
#endif

    /* return the new socket fd */
    return sockfd;
}

#endif
