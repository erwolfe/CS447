#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <string.h>
#define PORT 9080

using namespace std;

int main(int argc, char const *argv[]){
  char buffer[1024] = {0};
  struct sockaddr_in my_addr, client_addr;
  unsigned int socket_id, child_sock;
  int bindStatus, listenStatus, addrLen = sizeof(struct sockaddr_in);

  //Check to see if the socket can be created
  if ((socket_id = socket(AF_INET, SOCK_STREAM, 0)) == 0){
    cerr << "Socket was not initialized" << endl;
    exit(1);
  }

  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(PORT);
  my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  //Check to see if the AF, IP, and Port can be bound to the socket
  if ((bindStatus = bind(socket_id, (struct sockaddr *)&my_addr, sizeof(my_addr))) < 0){
    cerr << "Socket could not be bound" << endl;
    exit(1);
  }

  if ((listenStatus = listen(socket_id, 3)) < 0){
    cerr << "Error with listen function" << endl;
    exit(1);
  }

  while (true) {
    child_sock = accept(socket_id, (struct sockaddr *)&client_addr, (socklen_t*)&addrLen);
    char *message = "Get HTTP/1.1\r\n";
    send(child_sock, message, strlen(message), 0);
    recv(child_sock, buffer, sizeof(buffer)-1, 0);
    cout << buffer << endl;
    close(child_sock);
  }

  return 0;
}