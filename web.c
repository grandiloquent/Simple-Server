// -lws2_32 -lpthread

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
#  include <pthread.h>
#  include <dirent.h>
#  include <sys/stat.h>

#  define ISspace(x) isspace((int)(x))
#  define SERVER_STRING "Server: simpleServer\r\n"

void accept_request(int client);
void bad_request(int client);
void cannot_execute(int client);
void cat(int client, FILE *resource);
void error_die(const char *sc);
int get_line(int sock, char *buf, int size);
void headers(int client, const char *filename);
void not_found(int client);
void serve_file(int client, const char *filename);
int startup(const char *ip, u_short *port);
void unimplemented(int client);

void accept_request(int client) {
  char buf[1024];
  int numchars;
  char method[255];
  char url[255];
  char path[512];
  char agent[512];

  size_t i, j;
  struct stat st;

  char *query_string = NULL;
  numchars = get_line(client, buf, sizeof(buf));
  i = 0;
  j = 0;
  while (!ISspace(buf[j]) && (i < sizeof(method) - 1)) {
    method[i] = buf[j];
    i++;
    j++;
  }
  method[i] = '\0';
  if (strcasecmp(method, "GET") && strcasecmp(method, "POST")) {
    unimplemented(client);
    return;
  }
  // if (strcasecmp(method, "POST") == 0) cgi = 1;
  i = 0;
  while (ISspace(buf[j]) && (j < sizeof(buf))) j++;
  while (!ISspace(buf[j]) && (i < sizeof(url) - 1) && (j < sizeof(buf))) {
    url[i] = buf[j];
    i++;
    j++;
  }
  url[i] = '\0';
  if (strcasecmp(method, "GET") == 0) {
    query_string = url;
    while ((*query_string != '?') && (*query_string != '\0')) query_string++;
    if (*query_string == '?') {
      *query_string = '\0';
      query_string++;
    }
  }
  sprintf(path, "htdocs%s", url);
  if (path[strlen(path) - 1] == '/') strcat(path, "index.html");
  while ((numchars > 0) && strcmp("\n", buf)) {
    numchars = get_line(client, buf, sizeof(buf));
    if (strncasecmp(buf, "User-Agent", 10) == 0) {
      if (strlen(buf) > 12) continue;
      i = 0;
      j = 12;
      while (buf[j] != '\n' && (j < sizeof(agent) - 1)) {
        agent[i] = buf[j];
        i++;
        j++;
      }
      printf("%s\n", agent);
    }
  }
  if (stat(path, &st) == -1) {
    not_found(client);
  } else {
    printf("%d\n", st.st_mode);

    if ((st.st_mode & S_IFMT) == S_IFDIR) strcat(path, "/index.html");

    serve_file(client, path);
  }
  CLOSESOCKET(client);
}
void bad_request(int client) {
  char buf[1024];
  sprintf(buf, "HTTP/1.0 400 BAD REQUEST\r\n");
  send(client, buf, sizeof(buf), 0);
  sprintf(buf, "Content-type: text/html\r\n");
  send(client, buf, sizeof(buf), 0);
  sprintf(buf, "\r\n");
  send(client, buf, sizeof(buf), 0);
  sprintf(buf, "<P>Your browser sent a bad request, ");
  send(client, buf, sizeof(buf), 0);
  sprintf(buf, "such as a POST without a Content-Length.\r\n");
  send(client, buf, sizeof(buf), 0);
}
void cannot_execute(int client) {
  char buf[1024];
  sprintf(buf, "HTTP/1.0 500 Internal Server Error\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "Content-type: text/html\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "<P>Error prohibited CGI execution.\r\n");
  send(client, buf, strlen(buf), 0);
}
void cat(int client, FILE *resource) {
  char buf[1024];
  fgets(buf, sizeof(buf), resource);
  while (!feof(resource)) {
    // https://docs.microsoft.com/en-us/windows/desktop/api/winsock2/nf-winsock2-send
    if (send(client, buf, strlen(buf), 0) == -1) return;
    fgets(buf, sizeof(buf), resource);
  }
}
void error_die(const char *sc) {
  perror(sc);
  exit(1);
}

int get_line(int sock, char *buf, int size) {
  int i = 0;
  char c = '\0';
  int n;
  while ((i < size - 1) && (c != '\n')) {
    n = recv(sock, &c, 1, 0);
    if (n > 0) {
      if (c == '\r') {
        // https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-recv
        n = recv(sock, &c, 1, MSG_PEEK);
        if ((n > 0) && (c == '\n'))
          recv(sock, &c, 1, 0);
        else
          c = '\n';
      }
      buf[i] = c;
      i++;
    } else
      c = '\n';
  }
  buf[i] = '\0';
  return (i);
}
void headers(int client, const char *filename) {
  char buf[1024];
  (void)filename;
  strcpy(buf, "HTTP/1.0 200 OK\r\n");
  send(client, buf, strlen(buf), 0);
  strcpy(buf, SERVER_STRING);
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "Content-Type: text/html\r\n");
  send(client, buf, strlen(buf), 0);
  strcpy(buf, "\r\n");
  send(client, buf, strlen(buf), 0);
}
void not_found(int client) {
  /*
  HTTP/1.0 404 NOT FOUND
  Content-Type: text/html
  Content-Length: 499
  */
  char header[] = {72,  84,  84,  80,  47,  49,  46,  48,  32,  52,  48,  52,
                   32,  78,  79,  84,  32,  70,  79,  85,  78,  68,  13,  10,
                   67,  111, 110, 116, 101, 110, 116, 45,  84,  121, 112, 101,
                   58,  32,  116, 101, 120, 116, 47,  104, 116, 109, 108, 13,
                   10,  67,  111, 110, 116, 101, 110, 116, 45,  76,  101, 110,
                   103, 116, 104, 58,  32,  52,  57,  57,  13,  10,  13,  10};
  char html[] = {
      60,  33,  68,  79,  67,  84,  89,  80,  69,  32,  104, 116, 109, 108, 62,
      60,  104, 116, 109, 108, 32,  108, 97,  110, 103, 61,  34,  101, 110, 34,
      62,  60,  104, 101, 97,  100, 62,  60,  109, 101, 116, 97,  32,  99,  104,
      97,  114, 115, 101, 116, 61,  34,  85,  84,  70,  45,  56,  34,  47,  62,
      60,  109, 101, 116, 97,  32,  110, 97,  109, 101, 61,  34,  118, 105, 101,
      119, 112, 111, 114, 116, 34,  32,  99,  111, 110, 116, 101, 110, 116, 61,
      34,  119, 105, 100, 116, 104, 61,  100, 101, 118, 105, 99,  101, 45,  119,
      105, 100, 116, 104, 44,  32,  105, 110, 105, 116, 105, 97,  108, 45,  115,
      99,  97,  108, 101, 61,  49,  46,  48,  34,  47,  62,  60,  109, 101, 116,
      97,  32,  104, 116, 116, 112, 45,  101, 113, 117, 105, 118, 61,  34,  88,
      45,  85,  65,  45,  67,  111, 109, 112, 97,  116, 105, 98,  108, 101, 34,
      32,  99,  111, 110, 116, 101, 110, 116, 61,  34,  105, 101, 61,  101, 100,
      103, 101, 34,  47,  62,  60,  116, 105, 116, 108, 101, 62,  78,  111, 116,
      32,  70,  111, 117, 110, 100, 60,  47,  116, 105, 116, 108, 101, 62,  60,
      115, 116, 121, 108, 101, 62,  104, 116, 109, 108, 123, 102, 111, 110, 116,
      45,  115, 105, 122, 101, 58,  49,  48,  112, 120, 125, 46,  99,  111, 110,
      116, 97,  105, 110, 101, 114, 123, 100, 105, 115, 112, 108, 97,  121, 58,
      102, 108, 101, 120, 59,  97,  108, 105, 103, 110, 45,  105, 116, 101, 109,
      115, 58,  99,  101, 110, 116, 101, 114, 59,  106, 117, 115, 116, 105, 102,
      121, 45,  99,  111, 110, 116, 101, 110, 116, 58,  99,  101, 110, 116, 101,
      114, 59,  102, 108, 101, 120, 45,  102, 108, 111, 119, 58,  99,  111, 108,
      117, 109, 110, 125, 104, 49,  123, 102, 111, 110, 116, 45,  115, 105, 122,
      101, 58,  49,  50,  114, 101, 109, 125, 60,  47,  115, 116, 121, 108, 101,
      62,  60,  98,  111, 100, 121, 62,  60,  100, 105, 118, 32,  99,  108, 97,
      115, 115, 61,  34,  99,  111, 110, 116, 97,  105, 110, 101, 114, 34,  62,
      60,  104, 49,  62,  52,  48,  52,  60,  47,  104, 49,  62,  60,  112, 62,
      84,  104, 101, 32,  115, 101, 114, 118, 101, 114, 32,  99,  111, 117, 108,
      100, 32,  110, 111, 116, 32,  102, 117, 108, 102, 105, 108, 108, 32,  121,
      111, 117, 114, 32,  114, 101, 113, 117, 101, 115, 116, 32,  98,  101, 99,
      97,  117, 115, 101, 32,  116, 104, 101, 32,  114, 101, 115, 111, 117, 114,
      99,  101, 32,  115, 112, 101, 99,  105, 102, 105, 101, 100, 32,  105, 115,
      32,  117, 110, 97,  118, 97,  105, 108, 97,  98,  108, 101, 32,  111, 114,
      32,  110, 111, 110, 101, 120, 105, 115, 116, 101, 110, 116, 46,  60,  47,
      100, 105, 118, 62};
  send(client, header, sizeof(header), 0);
  send(client, html, sizeof(html), 0);
}
void serve_file(int client, const char *filename) {
  printf("%s\n", "serve_file");
  FILE *resource = NULL;
  int numchars = 1;
  char buf[1024];
  buf[0] = 'A';
  buf[1] = '\0';
  while ((numchars > 0) && strcmp("\n", buf))
    numchars = get_line(client, buf, sizeof(buf));
  resource = fopen(filename, "r");
  if (resource == NULL) {
    not_found(client);
  } else {
    headers(client, filename);
    cat(client, resource);
  }
  fclose(resource);
}
int startup(const char *ip, u_short *port) {
  int httpd = 0;
  // https://docs.microsoft.com/en-us/windows/desktop/api/ws2def/ns-ws2def-sockaddr_in
  struct sockaddr_in name;
  httpd = socket(PF_INET, SOCK_STREAM, 0);
  if (httpd == -1) error_die("socket");
  memset(&name, 0, sizeof(name));
  name.sin_family = AF_INET;
  name.sin_port = htons(*port);
  name.sin_addr.s_addr = inet_addr(ip);  // htonl(INADDR_ANY);
  if (bind(httpd, (struct sockaddr *)&name, sizeof(name)) < 0)
    error_die("bind");
  if (*port == 0) {
    int namelen = sizeof(name);
    // https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-getsockname
    if (getsockname(httpd, (struct sockaddr *)&name, &namelen) == -1)
      error_die("getsockname");
    *port = ntohs(name.sin_port);
  }
  if (listen(httpd, 5) < 0) error_die("listen");
  return (httpd);
}
void unimplemented(int client) {
  char buf[1024];
  sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, SERVER_STRING);
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "Content-Type: text/html\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "</TITLE></HEAD>\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "</BODY></HTML>\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "</BODY></HTML>\r\n");
  send(client, buf, strlen(buf), 0);
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
  int server_sock = -1;
  u_short port = 8089;
  int client_sock = -1;
  struct sockaddr_in client_name;
  int client_name_len = sizeof(client_name);
  pthread_t newthread;
  server_sock = startup("127.0.0.1", &port);
  while (1) {
    client_sock =
        accept(server_sock, (struct sockaddr *)&client_name, &client_name_len);
    if (client_sock == -1) error_die("accept");
    if (pthread_create(&newthread, NULL, accept_request, client_sock) != 0)
      perror("pthread_create");
  }
  CLOSESOCKET(1);
#  if defined(_WIN32)
  WSACleanup();
#  endif
  printf("Finished.\n");
  return 0;
}

#endif  // SINGLE_WEB_H

// http://localhost:8089/