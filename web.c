// -lws2_32

#ifndef SINGLE_WEB_H
#  define SINGLE_WEB_H
#  if defined(_WIN32)
#    ifndef _WIN32_WINNT
#      define _WIN32_WINNT 0x0600
#    endif
#    include <winsock2.h>
#    include <ws2tcpip.h>
#    pragma comment(lib, "ws2_32.lib")
#  else
#    include <sys/types.h>
#    include <sys/socket.h>
#    include <netinet/in.h>
#    include <arpa/inet.h>
#    include <netdb.h>
#    include <unistd.h>
#    include <errno.h>
#  endif
#  if defined(_WIN32)
#    define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#    define CLOSESOCKET(s) closesocket(s)
#    define GETSOCKETERRNO() (WSAGetLastError())
#  else
#    define ISVALIDSOCKET(s) ((s) >= 0)
#    define CLOSESOCKET(s) close(s)
#    define SOCKET int
#    define GETSOCKETERRNO() (errno)
#  endif
#  include <stdio.h>
#  include <string.h>
#  include <stdlib.h>

const char *get_content_type(const char *path) {
  const char *last_dot = strrchr(path, '.');
  if (last_dot) {
    if (strcmp(last_dot, ".css") == 0) return "text/css";
  }
  return "application/octet-stream";
}
SOCKET create_socket(const char *host, const char *port) {
  printf("Configuring local address...\n");
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  struct addrinfo *bind_address;
  getaddrinfo(host, port, &hints, &bind_address);
  printf("Creating socket...\n");
  SOCKET socket_listen;
  socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype,
                         bind_address->ai_protocol);
  if (!ISVALIDSOCKET(socket_listen)) {
    fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
    exit(1);
  }
  printf("Binding socket to local address...\n");
  if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) {
    fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
    exit(1);
  }
  freeaddrinfo(bind_address);
  printf("Listening...\n");
  if (listen(socket_listen, 10) < 0) {
    fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
    exit(1);
  }
  return socket_listen;
}
#  define MAX_REQUEST_SIZE 2047
struct client_info {
  socklen_t address_length;
  struct sockaddr_storage address;
  SOCKET socket;
  char request[MAX_REQUEST_SIZE + 1];
  char method[10];
  char uri[256];
  int received;
  struct client_info *next;
};
static struct client_info *clients = 0;
struct client_info *get_client(SOCKET s) {
  struct client_info *ci = clients;
  while (ci) {
    if (ci->socket == s) break;
    ci = ci->next;
  }
  if (ci) return ci;
  struct client_info *n =
      (struct client_info *)calloc(1, sizeof(struct client_info));
  if (!n) {
    fprintf(stderr, "Out of memory.\n");
    exit(1);
  }
  n->address_length = sizeof(n->address);
  n->next = clients;
  clients = n;
  return n;
}
void drop_client(struct client_info *client) {
  CLOSESOCKET(client->socket);
  struct client_info **p = &clients;
  while (*p) {
    if (*p == client) {
      *p = client->next;
      free(client);
      return;
    }
    p = &client->next;
  }
  fprintf(stderr, "drop_client not found.\n");
  exit(1);
}
const char *get_client_address(struct client_info *ci) {
  static char address_buffer[100];
  getnameinfo((struct sockaddr *)&ci->address, ci->address_length,
              address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
  return address_buffer;
}
fd_set wait_on_clients(SOCKET server) {
  fd_set reads;
  FD_ZERO(&reads);
  FD_SET(server, &reads);
  SOCKET max_socket = server;
  struct client_info *ci = clients;
  while (ci) {
    FD_SET(ci->socket, &reads);
    if (ci->socket > max_socket) max_socket = ci->socket;
    ci = ci->next;
  }
  if (select(max_socket + 1, &reads, 0, 0, 0) < 0) {
    fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
    exit(1);
  }
  return reads;
}
void send_400(struct client_info *client) {
  const char *c400 =
      "HTTP/1.1 400 Bad Request\r\n"
      "Connection: close\r\n"
      "Content-Length: 11\r\n\r\nBad Request";
  send(client->socket, c400, strlen(c400), 0);
  drop_client(client);
}
void send_404(struct client_info *client) {
  const char *c404 =
      "HTTP/1.1 404 Not Found\r\n"
      "Connection: close\r\n"
      "Content-Length: 9\r\n\r\nNot Found";
  send(client->socket, c404, strlen(c404), 0);
  drop_client(client);
}

void header(struct client_info *client) {
  const char *a = client->request;
  int c = 100, d = 0, e = 0, f = 0, i = 0, k = 0;
  char b[c + 1];
  char *h = client->method;
  char *j = client->uri;

  while (*a) {
    if (*a == '\r') {
      // terminate string
      b[d] = 0;
      // parsing method url
      if (!f) {
        for (int g = 0; g < c; g++) {
          if (!i) {  // parse method
            *h++ = b[g];
            if (b[g] == ' ') i = 1;
          } else if (!k) {  // parse uri
            *j++ = b[g];
            if (b[g] == ' ') k = 1;
          }
        }
        printf("%s %s\n", client->method, client->uri);

        f = 1;
      }
      //
      // reset index
      d = 0;
      // Checking if the pointer has reached the "\r\n\r\n"
      e = 0;
      while (*(++a) == '\n' || *a == 'r') e++;
      if (e == 3) break;
      //
      continue;
    }
    if (d < c) b[d++] = *a;
    a++;
  }
}
int main(int argc, char *argv[]) {
#  if defined(_WIN32)
  WSADATA d;
  if (WSAStartup(MAKEWORD(2, 2), &d)) {
    fprintf(stderr, "Failed to initialize.\n");
    return 1;
  }
#  else
#  endif

  SOCKET server = create_socket(0, "8080");
  while (1) {
    fd_set reads;
    reads = wait_on_clients(server);
    if (FD_ISSET(server, &reads)) {
      struct client_info *client = get_client(-1);
      client->socket = accept(server, (struct sockaddr *)&(client->address),
                              &(client->address_length));
      if (!ISVALIDSOCKET(client->socket)) {
        fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
      }
      printf("New connection from %s.\n", get_client_address(client));
    }
    struct client_info *client = clients;
    while (client) {
      if (FD_ISSET(client->socket, &reads)) {
        printf("123\n");

        if (MAX_REQUEST_SIZE == client->received) {
          send_400(client);
          continue;
        }
        int r = recv(client->socket, client->request + client->received,
                     MAX_REQUEST_SIZE - client->received, 0);
        if (r < 1) {
          printf("Unexpected disconnect from %s.\n",
                 get_client_address(client));
          drop_client(client);
        } else {
          client->received += r;
          client->request[client->received] = 0;
          header(client);
          char *q = strstr(client->request, "\r\n\r\n");
          if (q) {
            *q = 0;
            if (strncmp("GET /", client->request, 5)) {
              send_400(client);
            } else {
              char *path = client->request + 4;
              char *end_path = strstr(path, " ");
              if (!end_path) {
                send_400(client);
              } else {
                *end_path = 0;
              }
              printf("%s\n", path);
            }
          }
        }
      }
      client = client->next;
    }
  }
  printf("\nClosing socket...\n");
  CLOSESOCKET(server);
#  if defined(_WIN32)
  WSACleanup();
#  endif
  printf("Finished.\n");
  return 0;
}
#endif  // SINGLE_WEB_H

// http://localhost:8080/