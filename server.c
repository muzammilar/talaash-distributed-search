/* main.c */
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <pthread.h>
#include "message.h"
#include "my_util.h"
#include <vector>
#include <unistd.h>
#include <string>
#include <cstring>
#include <sys/types.h> // standard system types
#include <netinet/in.h> // Internet address structures
#include <sys/socket.h> // socket API
#include <arpa/inet.h>
#include <netdb.h> // host to IP resolution
#include <fstream>
#include <algorithm> 
using namespace std;
void sendMessageOrReply(char* senderAddress, unsigned short int senderPort, struct message* msg);

//globals
int global_client_id=1432;
pthread_mutex_t mtx_clients;
vector<client_info> my_clients;
vector<openned_file> my_openned_files;
//assignment is understood using Beej's guide
void printAndExit(string x){
		cout<<"usage: "<<x<<" <port>\n";
		exit(1);
}
//I have a condition that clients will come before workers

void sendCancelJob(unsigned int id){

}

void* serverKillerThread(void* xyc){

}

void* serverPingerThread(void* xyz){

}

void sendMessageOrReply(char* senderAddress, unsigned short int senderPort, struct message* msg){
	int sockfd;
	struct sockaddr_in reciever_addr;
	struct hostent* he;
	he=gethostbyname(senderAddress);
	sockfd = socket(PF_INET, SOCK_DGRAM, 0);//no error checking :P
	reciever_addr.sin_family=AF_INET;
	reciever_addr.sin_port=htons(senderPort);
	reciever_addr.sin_addr=*((struct in_addr *)he->h_addr);
	memset(&(reciever_addr.sin_zero), '\0', 8);
	char c[MAXMSGLEN];
	toBytes(msg,c);
	sendto(sockfd, c, MAXMSGLEN, 0,(struct sockaddr *)&reciever_addr, sizeof(struct sockaddr));
	close(sockfd);
}

//pinger, killer,job_assigner thread

void addMyClient(unsigned int id,message_and_sender* msg){
	struct client_info new_worker;
	new_worker.id=id;
	new_worker.port=msg->senderPort;
	new_worker.ip=msg->senderAddress;
	my_clients.push_back(new_worker);
	msg->msg->Client_ID=global_client_id;
	msg->msg->confirmation=1;
}

void removeMyFile(char* fileName, int cID){
	for (vector<openned_file>::iterator cli_it = my_openned_files.begin(); cli_it != my_openned_files.end();)
	{
		if(strcmp(fileName,cli_it->name)==0 && cID==cli_it->client_id){
			cout<<"Closing file for client!"<<endl;
			cli_it=my_openned_files.erase(cli_it);
		}
		else{
			 ++cli_it;
		}
	}
}


void* processMessage(void* msg_and_sender2){
	struct message_and_sender* msg_and_sender=(struct message_and_sender*) msg_and_sender2;
	cout<<"Message from "<<msg_and_sender->senderAddress<<","<<msg_and_sender->senderPort<<" recieved.\n";
	//print_message_function((void*) msg_and_sender->msg);
	struct message m1;		
	struct message* m2;
	if(msg_and_sender->msg->Command==JOIN_TO_SERVER){
		//now assign this guy a job!
		pthread_mutex_lock(&mtx_clients);//cout<<"locking jobs"<<endl;
		global_client_id+=114;
		//add this guy to your system.
		addMyClient(global_client_id,msg_and_sender);
		//reply that this guy has been added
		msg_and_sender->msg->confirmation=1;
		sendMessageOrReply(msg_and_sender->senderAddress,msg_and_sender->senderPort,msg_and_sender->msg);
		pthread_mutex_unlock(&mtx_clients);
	}
	else if (msg_and_sender->msg->Command==CREATE_FILE){
		cout<<"Creating File: "<<msg_and_sender->msg->File_Name<<endl;
		if(createFile(msg_and_sender->msg->File_Name)==1){
			cout<<"File Created!"<<endl;
			msg_and_sender->msg->confirmation=1;
			sendMessageOrReply(msg_and_sender->senderAddress,msg_and_sender->senderPort,msg_and_sender->msg);
			//tell others to create file as well
		}
		else{
			cout<<"File Creation Failed!"<<endl;			
			msg_and_sender->msg->confirmation=-1;
			sendMessageOrReply(msg_and_sender->senderAddress,msg_and_sender->senderPort,msg_and_sender->msg);			
		}
	}
	else if (msg_and_sender->msg->Command==OPEN_FILE){
		//now send a complete copy of the file!
		ifstream f(msg_and_sender->msg->File_Name);
	    if (f.good()) {
	        f.close();
	        // 
	        bool sorry=false;
        	for (vector<openned_file>::iterator iter = my_openned_files.begin(); iter != my_openned_files.end(); ++iter){
				//see if file is already open
				if(strcmp(msg_and_sender->msg->File_Name,iter->name)==0 && iter->filemode==WRITE_MODE && msg_and_sender->msg->File_Mode==WRITE_MODE){
					sorry=true;
				}
			}
			if(sorry){
				cout<<"Trying to write a locked file!"<<endl;
				msg_and_sender->msg->confirmation=-2;
			}
			else{
				//now add this file
				struct openned_file new_openned_file;
				strcpy(new_openned_file.name,msg_and_sender->msg->File_Name);
				new_openned_file.filemode=msg_and_sender->msg->File_Mode;//read or write
				new_openned_file.connectionmode=0;
	    		new_openned_file.client_id=msg_and_sender->msg->Client_ID;
	    		my_openned_files.push_back(new_openned_file);
				msg_and_sender->msg->confirmation=1;
				//=========================== read this file
				int begin,end;
				ifstream myfile(msg_and_sender->msg->File_Name);
				begin = (int)myfile.tellg();
				myfile.seekg (0, ios::end);
				end = (int)myfile.tellg();
				myfile.close();
		      ///////////////////////
	            myfile.open(msg_and_sender->msg->File_Name);
				int mybuflen;//we use a dynamic length since i think tcp waits for the whole packet to fill up
		        mybuflen=min(end-begin,MAX_FILESIZE_LIMIT);
		        myfile.read(msg_and_sender->msg->File_Data,mybuflen);
		        myfile.close();
			}
	    }
	    else{
			msg_and_sender->msg->confirmation=-1;
	    }
		sendMessageOrReply(msg_and_sender->senderAddress,msg_and_sender->senderPort,msg_and_sender->msg);			
	}
	else if (msg_and_sender->msg->Command==WRITE_FILE){
		cout<<"Updating File: "<<msg_and_sender->msg->File_Name<<endl;
		saveFile(msg_and_sender->msg->File_Name,msg_and_sender->msg->File_Data);

		if(msg_and_sender->msg->Query_Type==BLOCKING){
			//update others
        	for (vector<client_info>::iterator iter = my_clients.begin(); iter != my_clients.end(); ++iter){
				//see if file is already open
				if(iter->id!=msg_and_sender->msg->Client_ID){
		        	for (vector<openned_file>::iterator jiter = my_openned_files.begin(); jiter != my_openned_files.end(); ++jiter){
		        		if(iter->id==jiter->client_id){
							sendMessageOrReply((char*)((iter->ip).c_str()),iter->port,msg_and_sender->msg);
		        		}
		        	}
				}
			}
			sendMessageOrReply(msg_and_sender->senderAddress,msg_and_sender->senderPort,msg_and_sender->msg);			
		}
		else{//non blockingS
			sendMessageOrReply(msg_and_sender->senderAddress,msg_and_sender->senderPort,msg_and_sender->msg);			
			usleep(10000000);
			//update others.
        	for (vector<client_info>::iterator iter = my_clients.begin(); iter != my_clients.end(); ++iter){
				//see if file is already open
				if(iter->id!=msg_and_sender->msg->Client_ID){
		        	for (vector<openned_file>::iterator jiter = my_openned_files.begin(); jiter != my_openned_files.end(); ++jiter){
		        		if(iter->id==jiter->client_id){
							sendMessageOrReply((char*)((iter->ip).c_str()),iter->port,msg_and_sender->msg);
		        		}
		        	}
				}
			}

		}
	}
	else if (msg_and_sender->msg->Command==CLOSE_FILE){
		removeMyFile(msg_and_sender->msg->File_Name,msg_and_sender->msg->Client_ID);
		sendMessageOrReply(msg_and_sender->senderAddress,msg_and_sender->senderPort,msg_and_sender->msg);			
	}
	free(msg_and_sender->msg);
	free(msg_and_sender);
	return (void *)0;
}

int main(int argc, char *argv[])
{
	global_client_id=1432;
	unsigned short int port_num;//server port number. our port number is auto generated
	unsigned short int local_port;
	unsigned int query_type;
	char* port_num_char;
	char* server_hostname;
	struct hostent* server_ent;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	socklen_t addr_len;
	struct message *m1;
	pthread_t thread1;
	pthread_t pinger_thread;
	pthread_t killer_thread;
	int numbBytesRecieved;
	char recieve_buffer[MAXBUFLEN];
	int server_sockfd;
	m1 = (struct message *)NULL;
	/*
     * check command line arguments
     */
    string file_name(argv[0]);
     if(argc != 2){
		perror("Arguments");
		printAndExit(file_name);
	}
	port_num_char=argv[1];
	port_num=(short)(atoi(port_num_char));
	if(port_num> 65535){
		perror("Port Number");
		printAndExit(file_name);
	}
	if(port_num==0){
		cout<<"Sorry! We can't bind to a random port. Please specify one."<<endl;
		printAndExit(file_name);
	}
	//creating socket
	if ((server_sockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		printAndExit(file_name);
	}

	//send msg to server and wait for response
	server_addr.sin_family=AF_INET;
	server_addr.sin_port=htons((short)port_num);
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	memset(&(server_addr.sin_zero), '\0', 8); // zero the rest of the struct

	//bind to a local port
	while(bind(server_sockfd,(struct sockaddr *)&server_addr, sizeof(struct sockaddr))==-1){
		port_num+=1;
		server_addr.sin_port=htons((short)port_num);
	}
	//pthread_create(&pinger_thread,NULL,serverPingerThread,(void*)NULL);
	//pthread_create(&killer_thread,NULL,serverKillerThread,(void*)NULL);

	//cout<<"The local IP address is: "<<inet_ntoa(server_addr.sin_addr)<<endl;
	cout<<"The local port bound for connection: "<<port_num<<endl;
	//request and connect the server
	struct message_and_sender* msg_and_sender=(struct message_and_sender*)NULL;
	addr_len = sizeof(struct sockaddr);
	while(true){
		memset(recieve_buffer,'\0', MAXBUFLEN);
		if ((numbBytesRecieved=recvfrom(server_sockfd, recieve_buffer, MAXMSGLEN , 0,(struct sockaddr *)&client_addr, &addr_len)) == -1) {
			perror("Could not recieve incoming message:");
		}
		recieve_buffer[numbBytesRecieved]='\0';
		msg_and_sender= new message_and_sender();
		msg_and_sender->msg=toMessage(recieve_buffer);
		msg_and_sender->senderAddress=inet_ntoa(client_addr.sin_addr);
		msg_and_sender->senderPort=ntohs(client_addr.sin_port);
		pthread_create(&thread1,NULL,processMessage,(void*)msg_and_sender);
	}
	

	/* Deallocating memmory m1, m2 */
	free(m1);	
	close(server_sockfd);
	return 1;
}
