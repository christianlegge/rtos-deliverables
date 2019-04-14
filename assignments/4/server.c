#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/ip.h>

const char** usernames;
const char** rooms;
int numUsernames = 0;
int numRooms = 0;
int numRequests = 0;
pthread_t *threads;
int numThreads = 0;
int numUsers = 0;
int *connectedUsers;
int *roomsByUser;

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

union tlvVal parseTlv(char* params, int ind) {
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
        retval.stringVal = malloc(length);
        memcpy(retval.stringVal, value, length);
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

void *buildResHeader(int request, int rpc, int numparams, int flags, int bodyLength) {
    void *ptr = malloc(16);
    *(uint16_t*)ptr = request;
    *(uint8_t*)(ptr+2) = rpc;
    *(uint8_t*)(ptr+3) = numparams;
    *(uint32_t*)(ptr+4) = flags;
    *(uint32_t*)(ptr+8) = time(NULL);
    *(uint32_t*)(ptr+12) = bodyLength;
    return ptr;
}

void *buildEventHeader(int type, int flags, int n, int bodyLength) {
    void *header = malloc(12);
    *(uint8_t*)header = type;
    *(uint16_t*)(header+1) = flags;
    *(uint8_t*)(header+3) = n;
    *(uint32_t*)(header+4) = time(NULL);
    *(uint32_t*)(header+8) = bodyLength;
    
    return header;
}

void broadcastMessageToChannel(char *msg, int sender_id, int room_id) {
    const char* sendingUser = usernames[sender_id];
    void *eventParams = malloc(9 + 2 + strlen(sendingUser) + strlen(msg));
    int eParamLen = addTlv(eventParams, 0, 1, 2, &room_id);
    eParamLen = addTlv(eventParams, 1, 2, strlen(sendingUser), sendingUser);
    eParamLen = addTlv(eventParams, 2, 2, strlen(msg), msg);

    void *eventHeader = buildEventHeader(0, 1, 3, eParamLen);

    struct resHeader *res = malloc(sizeof(struct resHeader));
    res->request = 0;
    res->rpc = 255;
    res->n = 1;
    res->flags = 1;
    res->serverTimestamp = time(NULL);
    res->bodyLength = 3 + 12 + eParamLen;
    
    char* eventTlvInfo = malloc(3);
    *(uint8_t*)eventTlvInfo = 2;
    *(uint16_t*)(eventTlvInfo+1) = 12 + eParamLen;

    for (int i = 0; i < numUsers; i++) {
        if (i != sender_id && roomsByUser[i] == room_id) {
            send(connectedUsers[i], res, 16, 0);
            send(connectedUsers[i], eventTlvInfo, 3, 0);
            send(connectedUsers[i], eventHeader, 12, 0);
            send(connectedUsers[i], eventParams, eParamLen, 0);
        }
    }
    
    
}

void handleHello(int cli_fd, char *params) {
    struct resHeader *res = malloc(sizeof(struct resHeader));
    res->request = ++numRequests;
    res->rpc = 1;
    res->n = 1;
    res->flags = 0;
    res->serverTimestamp = time(NULL);
    res->bodyLength = 4;
    char* sendParams = malloc(4);
    addTlv(sendParams, 0, 1, 1, 1);
    send(cli_fd, res, sizeof(struct resHeader), 0);
    send(cli_fd, sendParams, 4, 0);



}
void handleLogin(int cli_fd, char *params) {
    int len = *(uint16_t*)(params+1);
    char *username = parseTlv(params, 0).stringVal;
    username = realloc(username, len+1);
    username[len] = 0;
    printf("Username: %s\n", username);
    numUsernames++;
    usernames = realloc(usernames, sizeof(const char*)*numUsernames);
    usernames[numUsernames-1] = username;
    

    struct resHeader *res = malloc(sizeof(struct resHeader));
    res->request = ++numRequests;
    res->rpc = 1;
    res->n = 1;
    res->flags = 0;
    res->serverTimestamp = time(NULL);
    res->bodyLength = 7;
    char* sendParams = malloc(7);
    uint8_t *tag = sendParams;
    uint16_t *length = sendParams+1;
    uint32_t *value = sendParams+3;
    *tag = 1;
    *length = 4;
    *value = numUsernames;
    send(cli_fd, res, sizeof(struct resHeader), 0);
    send(cli_fd, sendParams, 7, 0);



}

void handleJoinRoom(int cli_fd, char *params) {
  char *roomName = parseTlv(params, 0).stringVal;
  printf("joining room %s\n", roomName);
  int roomId = 65535;
  for (int i = 0; i < numRooms; i++) {
    if (strcmp(rooms[i],roomName) == 0) {
      roomId = i;
    }
  }
  if (roomId != 65535) {
    for (int i = 0; i < numUsers; i++) {
      if (connectedUsers[i] == cli_fd) {
        roomsByUser[i] = roomId;
      }
    }
  }

  void *sendingParams = malloc(5);
  addTlv(sendingParams, 0, 1, 2, &roomId);
  void *header = buildResHeader(0, 6, 1, 0, 5);
  send(cli_fd, header, 16, 0);
  send(cli_fd, sendingParams, 5, 0);
}

void handleMessage(int cli_fd, char *params) {
    int sendingUserFd;
    for (int i = 0; i < numUsers; i++) {
        if (cli_fd == connectedUsers[i]) {
          sendingUserFd = i;
          break;
        }
    }
    int room_id = *(uint16_t*)(params+3);
    int msgLen = *(uint16_t*)(params+6);
    char* msg = malloc(msgLen);
    memcpy(msg, params+8, msgLen);
    printf("Message to %d: %s\n", room_id, msg); 
    broadcastMessageToChannel(msg, sendingUserFd, room_id);
}

void handleListRooms(int cli_fd, char *params) {
    void *sendingParams = NULL;
    int sentParamLen = 0;
    for (int i = 0; i < numRooms; i++) {
        sendingParams = realloc(sendingParams, sentParamLen + strlen(rooms[i]) + 3);
        sentParamLen = addTlv(sendingParams, i, 2, strlen(rooms[i]), rooms[i]);
    }
    void *header = buildResHeader(0, 4, numRooms, 0, sentParamLen);
    send(cli_fd, header, 16, 0);
    send(cli_fd, sendingParams, sentParamLen, 0);
}

void handleListUsers(int cli_fd, char *params) {
    int roomId = -1;
    int userId = -1;
    for (int i = 0; i < numUsers; i++) {
      if (connectedUsers[i] == cli_fd) {
        userId = i;
      }
    }
    roomId = roomsByUser[userId];
    void *sendingParams = NULL;
    int sentParamLen = 0;
    int numparams = 0;
    for (int i = 0; i < numUsers; i++) {
      if (roomsByUser[i] == roomId) {
        sendingParams = realloc(sendingParams, sentParamLen + strlen(usernames[i]) + 3);
        sentParamLen = addTlv(sendingParams, i, 2, strlen(usernames[i]), usernames[i]);
        numparams++;
      }
    }
    void *header = buildResHeader(0, 5, numparams, 0, sentParamLen);
    send(cli_fd, header, 16, 0);
    send(cli_fd, sendingParams, sentParamLen, 0);
}

void handleCreateRoom(int cli_fd, char *params) {
  char *roomName = parseTlv(params, 0).stringVal;
  printf("creating room %s\n", roomName);
  numRooms++;
  rooms = realloc(rooms, sizeof(char*)*numRooms);
  rooms[numRooms-1] = strdup(roomName);
  void *sendingParams = malloc(5);
  int newId = numRooms - 1;
  addTlv(sendingParams, 0, 1, 2, &newId);
  void *header = buildResHeader(0, 6, 1, 0, 5);
  send(cli_fd, header, 16, 0);
  send(cli_fd, sendingParams, 5, 0);
}

void recvReqRpc(int cli_fd) {
    struct reqHeader *buf = malloc(16);
    
    recv(cli_fd, (void*)buf, 16, MSG_WAITALL);
        
    void *params;
    if (buf->bodyLength > 0) {
        params = malloc(buf->bodyLength);
        recv(cli_fd, params, buf->bodyLength, MSG_WAITALL);
    }
    
    switch (buf->rpc) {
        case 0:
            printf("Handling hello\n");
            handleHello(cli_fd, params);
            break;
        case 1:
            printf("Handling login\n");
            handleLogin(cli_fd, params);
            break;
        case 2:
            printf("Handling join_room\n");
            handleJoinRoom(cli_fd, params);
            break;
        case 3:
            printf("Handling message\n");
            handleMessage(cli_fd, params);
            break;
        case 4:
            printf("Handling list_rooms\n");
            handleListRooms(cli_fd, params);
            break;
        case 5:
            printf("Handling list_users\n");
            handleListUsers(cli_fd, params);
            break;
        case 6:
            printf("Handling create_room\n");
            handleCreateRoom(cli_fd, params);
            break;
        default:
            printf("Invalid rpc\n");
            break;
    }
    
}

void *newThreadHandler(void *cli_fd) {
    printf("New thread receiving on fd %d\n", *(int*)cli_fd);
    while(1) {
        recvReqRpc(*(int*)cli_fd);
    }
}

int main() {

    rooms = realloc(rooms, sizeof(const char*) * (++numRooms));
    rooms[0] = "general";
    
    printf("Rooms: ");
    for (int i = 0; i < numRooms; i++) {
        printf("%s ", rooms[i]);
    }
    printf("\n");


    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Error creating socket: %d\n", errno);
        return -1;
    }

    struct sockaddr_in *addr = malloc(sizeof(struct sockaddr_in));

    addr->sin_family = AF_INET;
    addr->sin_port = htons(8894);
    addr->sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int binding = bind(sockfd, (struct sockaddr*)addr, sizeof(struct sockaddr_in));
    if (binding == -1) {
        printf("Error binding socket: %d\n", errno);
        return -1;
    }

    int listener = listen(sockfd, 50);
    if (listener == -1) {
        printf("Error listening on socket\n");
        return -1;
    }

    struct sockaddr_in *cli_addr = malloc(sizeof(struct sockaddr_in));
    socklen_t *cli_len = malloc(sizeof(socklen_t));
    *cli_len = sizeof(struct sockaddr_in);
    while(1) {
        printf("Waiting for connection from client\n");
        int cli_fd = accept(sockfd, (struct sockaddr*)cli_addr, cli_len);
        if (cli_fd == -1) {
          printf("Error accepting client connection: %d (%s)\n", errno, strerror(errno));
          continue;
        }
        printf("Connection accepted with fd %d\n", cli_fd);
        connectedUsers = realloc(connectedUsers, sizeof(int*)*(++numUsers));
        connectedUsers[numUsers-1] = cli_fd;
        roomsByUser = realloc(roomsByUser, sizeof(int*)*numUsers);
        roomsByUser[numUsers-1] = 0;
        threads = realloc(threads, sizeof(pthread_t*)*(++numThreads));
        pthread_create(threads+(numThreads-1), NULL, newThreadHandler, &cli_fd);
    }
}
