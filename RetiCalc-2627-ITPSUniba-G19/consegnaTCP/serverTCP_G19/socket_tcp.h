#ifndef SOCKET_TCP_H
#define SOCKET_TCP_H

#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET socket_tcp;
typedef int socket_length;
#define SOCKET_INVALID INVALID_SOCKET
#define CLOSE_SOCKET closesocket
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>
typedef int socket_tcp;
typedef socklen_t socket_length;
#define SOCKET_INVALID (-1)
#define CLOSE_SOCKET close
#endif

#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

enum { LINE_CAPACITY = 128 };

static int initialize_sockets(void)
{
#ifdef _WIN32
    WSADATA data;
    return WSAStartup(MAKEWORD(2, 2), &data) == 0;
#else
    return signal(SIGPIPE, SIG_IGN) != SIG_ERR;
#endif
}

static void cleanup_sockets(void)
{
#ifdef _WIN32
    WSACleanup();
#endif
}

static int socket_call_interrupted(void)
{
#ifdef _WIN32
    return WSAGetLastError() == WSAEINTR;
#else
    return errno == EINTR;
#endif
}

static int send_line(socket_tcp sock, const char *line)
{
    int remaining = (int)strlen(line);
    while (remaining > 0) {
        int sent = (int)send(sock, line, remaining, 0);
        if (sent < 0 && socket_call_interrupted())
            continue;
        if (sent <= 0)
            return 0;
        line += sent;
        remaining -= sent;
    }
    return 1;
}

/* TCP è un flusso. Il terminatore LF delimita ogni messaggio. */
static int receive_line(socket_tcp sock, char line[LINE_CAPACITY])
{
    size_t length = 0;
    while (length < LINE_CAPACITY - 1) {
        char byte;
        int received = (int)recv(sock, &byte, 1, 0);
        if (received < 0 && socket_call_interrupted())
            continue;
        if (received <= 0 || byte == '\0')
            return 0;
        if (byte == '\n') {
            line[length] = '\0';
            return 1;
        }
        line[length++] = byte;
    }
    return 0;
}

static int parse_integers(const char *input, int32_t values[], size_t count)
{
    for (size_t i = 0; i < count; ++i) {
        char *end;
        errno = 0;
        intmax_t value = strtoimax(input, &end, 10);
        if (end == input || errno == ERANGE ||
            value < INT32_MIN || value > INT32_MAX ||
            (*end && !isspace((unsigned char)*end)))
            return 0;
        values[i] = (int32_t)value;
        input = end;
    }
    while (isspace((unsigned char)*input))
        ++input;
    return *input == '\0';
}

#endif
