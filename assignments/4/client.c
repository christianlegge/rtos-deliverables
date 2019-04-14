#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/ip.h>

uint16_t currentRoom = 0;
int sockfd;
pthread_mutex_t recvMutex = PTHREAD_MUTEX_INITIALIZER;
int requestId = 0;
char username[256];
int sessionId = 0;

struct reqHeader {
    uint32_t session;
    uint16_t request;
    uint8_t rpc;
    uint8_t numparams;
    uint32_t clientTimestamp;
    uint32_t bodyLength;
};

struct resHeader {
    uint16_t request;
    uint8_t rpc;
    uint8_t n;
    uint32_t flags;
    uint32_t serverTimestamp;
    uint32_t bodyLength;
};

union tlvVal {
  char boolVal;
  uint64_t intVal;
  char *stringVal;
};

void *buildReqHeader(int rpc, int numparams, int bodyLength) {
    void *ptr = malloc(16);
    *(uint32_t*)ptr = sessionId;
    *(uint16_t*)(ptr+4) = ++requestId;
    *(uint8_t*)(ptr+6) = rpc;
    *(uint8_t*)(ptr+7) = numparams;
    *(uint32_t*)(ptr+8) = time(NULL);
    *(uint32_t*)(ptr+12) = bodyLength;
    return ptr;
}

union tlvVal parseTlv(const void* params, int ind) {
  int i = 0;
  int seek = 0;
  void *value;
  union tlvVal retval;
  while (i <= ind) {
    int tag = *(uint8_t*)(params+seek);
    seek += 1;
    int length = *(uint16_t*)(params+seek);
    seek += 2;
    value = (void*)(params+seek);
    seek += length;
    if (i++ == ind) {
      if (tag == 0) {
        retval.boolVal = *(char*)value;
      }
      else if (tag == 1) {
        if (length == 1) {
          retval.intVal = *(uint8_t*)value;
        }
        else if (length == 2) {
          retval.intVal = *(uint16_t*)value;
        }
        else if (length == 4) {
          retval.intVal = *(uint32_t*)value;
        }
        else if (length == 8) {
          retval.intVal = *(uint64_t*)value;
        }
      }
      else if (tag == 2) {
        retval.stringVal = malloc(length+1);
        memcpy(retval.stringVal, value, length);
        retval.stringVal[length] = 0;
      }
      return retval;
    }
  }
}

size_t addTlv(void *addr, int pos, int tag, int length, const void* value) {
    int i = 0;
    int seek = 0;
    while (i < pos) {
        int length = *(uint16_t*)(addr+seek+1);
        seek += 3+length;
        i++;
    }
    *(uint8_t*)(addr+seek) = tag;
    *(uint16_t*)(addr+seek+1) = length;
    memcpy(addr+seek+3, value, length);
    return seek+3+length;
}

void *sendReqRpc(int sockfd, int rpc, int p, int bodyLength, char *params, char wait) {
    struct reqHeader *header = buildReqHeader(rpc, p, bodyLength);
    pthread_mutex_lock(&recvMutex);
    send(sockfd, header, 16, 0);
    send(sockfd, params, bodyLength, 0);
    // Receive response
    if (!wait) {
        pthread_mutex_unlock(&recvMutex);
        return NULL;
    }
    void *buf = malloc(16);
    recv(sockfd, buf, 16, MSG_WAITALL);
    int resParamLen = ((struct resHeader*)buf)->bodyLength;

    buf = realloc(buf, 16+resParamLen);

    recv(sockfd, buf+16, resParamLen, MSG_WAITALL);

    pthread_mutex_unlock(&recvMutex);
    
    return buf;
}

void sendHello() {
  void *params = malloc(strlen("assign4-chat") + 3 + 3 + 1);
  addTlv(params, 0, 2, strlen("assign4-chat"), "assign4-chat");
  int paramLen = addTlv(params, 1, 1, 1, 1);
  void *response = sendReqRpc(sockfd, 0, 2, paramLen, params, 1);
  assert(parseTlv(response+16, 0).intVal == 1);
}

int sendLogin(const char *username) {
    void* params = malloc(strlen(username) + 3);
    addTlv(params, 0, 2, strlen(username), username);
    void *response = sendReqRpc(sockfd, 1, 1, 3 + strlen(username), params, 1);
    free(params);
    if (((struct resHeader*)response)->bodyLength > 0) {
      int session = parseTlv(response+16, 0).intVal;
      if (session <= 0) {
        free(response);
        return -1;
      }
      sessionId = session;
      free(response);
      return 0;
    }
    free(response);
    return -1;
}

int listRooms() {
    void *response = sendReqRpc(sockfd, 4, 0, 0, NULL, 1);
    void *params = response+16;
    struct resHeader *header = (struct resHeader*)response;
    if (header->bodyLength <= 0) {
      return -1;
    }

    printf("Available rooms: ");
    for (int i = 0; i < header->n; i++) {
        printf("#%s ", parseTlv(params, i).stringVal);
    }
    printf("\n");
    return 0;
}

int listUsers() {
    void *response = sendReqRpc(sockfd, 5, 0, 0, NULL, 1);
    void *params = response+16;
    struct resHeader *header = (struct resHeader*)response;
    if (header->bodyLength <= 0) {
      return -1;
    }

    printf("Users in this room: ");
    for (int i = 0; i < header->n; i++) {
        printf("%s ", parseTlv(params, i).stringVal);
    }
    printf("\n");
    return 0;
}

int createRoom(char *name) {
  printf("to be created: %s\n", name);
  void *params = malloc(3+strlen(name));
  int bodyLength = addTlv(params, 0, 2, strlen(name), name);
  void *response = sendReqRpc(sockfd, 6, 1, bodyLength, params, 1);
  struct resHeader *header = (struct resHeader*)response;
  if (header->bodyLength <= 0) {
    return -1;
  }

  printf("Created room with id %d\n", parseTlv(response+16, 0).intVal);

  return 0;
}

void sendMessage(const char* msg) {
    void* params = malloc(strlen(msg) + 3 + 5);
    int bodyLength = addTlv(params, 0, 1, 2, &currentRoom);
    bodyLength = addTlv(params, 1, 2, strlen(msg), msg);
    strcpy(params+8, msg);
    sendReqRpc(sockfd, 3, 2, strlen(msg) + 8, params, 0);
}

void joinRoom(const char *name) {
  printf("joining: %s\n", name);
  void *params = malloc(3+strlen(name));
  int bodyLength = addTlv(params, 0, 2, strlen(name), name);
  void *response = sendReqRpc(sockfd, 2, 1, bodyLength, params, 1);
  struct resHeader *header = (struct resHeader*)response;
  if (header->bodyLength <= 0) {
    return -1;
  }
  
  int newId = parseTlv(response+16, 0).intVal;
  
  if (newId == 65535) {
    printf("Room #%s does not exist.\n", name);
    return;
  }
  
  currentRoom = parseTlv(response+16, 0).intVal;

  printf("Joined room with id %d\n", parseTlv(response+16, 0).intVal);
}

void *handleEvents(void *_) {
    struct resHeader *buf = malloc(16);
    while(1) {
        int recvLen = 0;
        do {
            pthread_mutex_lock(&recvMutex);
            recvLen = recv(sockfd, buf, 16, MSG_DONTWAIT);
            pthread_mutex_unlock(&recvMutex);
        } while(recvLen <= 0);
        if (buf->request == 0 && buf->rpc == 255) {
            char* params = malloc(buf->bodyLength);
            do {
                recvLen = recv(sockfd, params, buf->bodyLength, MSG_PEEK);
            } while(recvLen < buf->bodyLength);
            
            if (*(uint8_t*)(params+3) == 0) {
                char* sendingUser = parseTlv(params+15, 1).stringVal;
                char* sentMsg = parseTlv(params+15, 2).stringVal;
                printf("\r<%s> %s\n<%s> ", sendingUser, sentMsg, username);
                fflush(stdout);
            }
        }
    }
}

int main() {
    char usernameSet = 0;
    
    while (!usernameSet) {
        printf("Enter username:\n");
        fgets(username, 256, stdin);
        username[strcspn(username, "\n")] = 0;
        char foundError = 0;
        for (int i = 0; i < 256; i++) {
            if (username[i] == 0) {
                break;
            }
            if (!isalpha(username[i]) && !isdigit(username[i]) && username[i] != '_') {
                printf("Invalid username. Alphanumeric + underscores only.\n");
                foundError = 1;
                break;
            }
        }
        if (!foundError) {
            usernameSet = 1;
        }
    }
    printf("Username set to %s.\n", username);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Error creating socket\n");
        return -1;
    }

    struct sockaddr_in *addr = malloc(sizeof(struct sockaddr_in));

    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr->sin_port = htons(8894);

    int connection = connect(sockfd, (struct sockaddr*)addr, sizeof(struct sockaddr_in));
    if (connection == -1) {
        return -1;
    }
    else {
    }


    // Send login RPC
    if (sendLogin(username) == -1) {
        printf("Error getting session\n");
    }

    printf("Joined room #general (Session ID %d)\n", sessionId);

    pthread_t eventListener;
    pthread_create(&eventListener, NULL, handleEvents, NULL);


    char *buf = malloc(256);
    while (1) {
        printf("<%s> ", username);
        fflush(stdout);
        fgets(buf, 256, stdin);
        buf[strcspn(buf, "\n")] = 0;
        if (buf[0] == '/') {
          const char *command = strtok(buf, " ")+1;
          if (strcmp(command, "hello") == 0) {
            sendHello();
          }
          else if (strcmp(command, "join") == 0) {
              command = strtok(NULL, " ");
              joinRoom(command);
          }
          else if (strcmp(command, "list") == 0) {
              listRooms();
          }
          else if (strcmp(command, "create") == 0) {
              createRoom(strtok(NULL, " "));
          }
          else if (strcmp(command, "users") == 0) {
              listUsers();
          }
          else if (command[0] == '/') {
            sendMessage(buf+1);
          }
          else {
            printf("Command %s not found.\n", command);
          }
        }
        else {
          sendMessage(buf);
        }
        //sendReqRpc(sockfd, 0, 1, 1, 0, 0, NULL);
        //send(sockfd, buf, strlen(buf), 0);
    }

}
