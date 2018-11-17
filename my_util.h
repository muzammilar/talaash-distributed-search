#ifndef MY_UTIL_H
#define MY_UTIL_H

#include "message.h"
#include <string.h>
#include <string>
#include <iostream>
#include <fstream>

#define MAXMSGLEN         16452
#define MAXBUFLEN              16453

struct message_and_sender{
	struct message* msg;
	char* senderAddress;
	unsigned short int senderPort;
};

struct job{
    unsigned int requester_id;
    unsigned int worker_id;
    message* msg;
};

struct client_info{
    unsigned int id;
    unsigned int port;
    std::string ip;
};

struct openned_file{
    char name[MAX_FILENAME_LEN];

    int filemode;//read or write
    int connectionmode;
    int client_id;
};

//unsigned int getFreePort(unsigned int my_port);
void toBytes(message *m,char * buf);
message* toMessage(char* rcvBuffer);
int createFile(char* fileName);
int saveFile(char* fileName,char mydata[]);
bool deleteFile(char* fileName);
#endif