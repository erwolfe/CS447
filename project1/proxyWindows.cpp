#include <stdio.h>          // Needed for printf()
#include <stdlib.h>         // Needed for exit()
#include <string.h>         // Needed for strcpy() and strlen()
#include <fcntl.h>          // Needed for file i/o constants
#include <sys/stat.h>       // Needed for file i/o constants

#include <stddef.h>         // Needed for _threadid
#include <process.h>        // Needed for _beginthread() and _endthread()
#include <io.h>             // Needed for open(), close(), and eof()
#include <windows.h>        // Needed for all Winsock stuff

// Defines
#define PROXYPORT 9080
#define BUFFERSIZE 4096
#define SERVERPORT 80
// HTTP response messages
#define OK_IMAGE    "HTTP/1.0 200 OK\nContent-Type:image/gif\n\n"
#define OK_TEXT     "HTTP/1.0 200 OK\nContent-Type:text/html\n\n"
#define NOTOK_404   "HTTP/1.0 404 Not Found\nContent-Type:text/html\n\n"
#define MESS_404    "<html><body><h1>FILE NOT FOUND</h1></body></html>"
// Prototypes
void *clientToServer(void *in_args); //Thread for sending information from the client to the server
void *serverToClient(void *in_args); //Thread for sending information from the server to the client

using namespace std;

int main(int argc, char const *argv[]) {
  WORD wVersionRequested = MAKEWORD(1,1);
  WSADATA wsaData;
  WSAStartup(wVersionRequested, &wsaData);
  unsigned int        proxySocket, clientSocket, serverSocket, serverSocketConnection;
  struct sockaddr_in  proxyAddress, clientAddress, serverAddress;
  struct in_addr      clientIPAddress, serverIPAddress;
  int                 addressLength;

  // Create a socket, fill-in address information, bind it, and then listen
  proxySocket = socket(AF_INET, SOCK_STREAM, 0);
  proxyAddress.sin_family = AF_INET;
  proxyAddress.sin_port = htons(PROXYPORT);
  proxyAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(proxySocket, (struct sockaddr *)&proxyAddress, sizeof(proxyAddress));
  listen(proxySocket, 100);

  // Main loop to accept, create new socket for server, connect to server, and start both threads
  while(1){
    addressLength = sizeof(clientAddress);
    if ((clientSocket = accept(proxySocket, (struct sockaddr *)&clientAddress, &addressLength)) == 0){
      printf("ERROR - Unable to create socket \n");
      exit(1);
    }

    /* Display the signature of the new client ----- */
    memcpy(&clientIPAddress, &clientAddress.sin_addr.s_addr, 4);
    printf("\n*** Profile for a connecting client ***\n");
    printf("IP address: %s\n", inet_ntoa(clientIPAddress));
    printf("Client-side Port: %d\n", ntohs(clientAddress.sin_port));
    printf("\n");

    // Create the server socket, fill-in address information, and then connect
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVERPORT);
    serverAddress.sin_addr.s_addr = inet_aton(“cs.siue.edu/~hfujino/CS447_F17_001”);
    serverSocketConnection = connect(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));

    // Setting up arguments for the threads (easier to pass socket information) and then start threads
    unsigned int threadArgs[2] = {clientSocket, serverSocket};
    if (_beginthread(cts, 4096, (void *)threadArgs) < 0){
      printf("ERROR - Unable to create client to server thread \n");
      exit(1);
    }
    if (_beginthread(stc, 4096, (void *)threadArgs) < 0){
      printf("ERROR - Unable to create client to server thread \n");
      exit(1);
    }
  }
  closesocket(proxySocket);
  WSACleanup();
  return 0;
}

void *clientToServer(void *in_args){
  unsigned int  clientSocket, serverSocket, fh, bufferLength, returnCode;
  char          inBuffer[BUFFERSIZE], outBuffer[BUFFERSIZE], *fileName;
  string        line;
  bool          connectionFlag = 1;

  clientSocket = *((unsigned int *) in_args[0]);
  serverSocket = *((unsigned int *) in_args[1]);

  while(connectionFlag == 1){
    returnCode = recv(clientSocket, inBuffer, BUFFERSIZE, 0);

    if (returnCode != -1){
      if (serverSocket == 0){
        strcpy(outBuffer, NOTOK_404);
        send(clientSocket, outBuffer, strlen(outBuffer), 0);
        strcpy(outBuffer, MESS_404);
        send(clientSocket, outBuffer, strlen(outBuffer), 0);
        connectionFlag = 0;
      }
      else{
        strtok(inBuffer, " ");
        file_name = strtok(NULL, " ");
        fileName = fileName + 1;
        send(serverSocket, fileName, strlen(fileName), 0);
      }
    }
  }
  _endthread();
}

void *serverToClient(void *in_args){
  unsigned int  clientSocket, serverSocket, fh, bufferLength, returnCode;
  char          inBuffer[BUFFERSIZE], outBuffer[BUFFERSIZE], *fileName;
  string        line;
  bool          connectionFlag = 1;

  clientSocket = *((unsigned int *) in_args[0]);
  serverSocket = *((unsigned int *) in_args[1]);
  while(connectionFlag == 1){
    returnCode = recv(serverSocket, inBuffer, BUFFERSIZE, 0);

    if (returnCode != -1){
      if (serverSocket == 0){
        strcpy(outBuffer, NOTOK_404);
        send(clientSocket, outBuffer, strlen(outBuffer), 0);
        strcpy(outBuffer, MESS_404);
        send(clientSocket, outBuffer, strlen(outBuffer), 0);
        connectionFlag = 0;
      }
      else{
        strcpy(outBuffer, inBuffer);
        send(clientSocket, outBuffer, strlen(outBuffer), 0);
      }
    }
  }
  closesocket(serverSocket);
  closesocket(clientSocket);
  _endthread();
}
