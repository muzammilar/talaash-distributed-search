/* main.c */
#include <stdio.h>
#include <stdlib.h>
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
using namespace std;

vector<openned_file> my_openned_files;

//assignment is understood using Beej's guide
void printAndExit(string x){
		cout<<"usage: "<<x<<" <server hostname> <port>\n";
		exit(-1);
}

struct message* sendMessageAndRecieveReply(char* senderAddress, unsigned short int senderPort, struct message* msg){
	int sockfd;
	struct sockaddr_in reciever_addr;
	struct hostent* he;
	he=gethostbyname(senderAddress);
	sockfd = socket(PF_INET, SOCK_DGRAM, 0);//no error checking, :P, cool
	reciever_addr.sin_family=AF_INET;
	reciever_addr.sin_port=htons(senderPort);
	reciever_addr.sin_addr=*((struct in_addr *)he->h_addr);
	memset(&(reciever_addr.sin_zero), '\0', 8);
	char c[MAXMSGLEN];
	char recieve_buffer[MAXBUFLEN];
	socklen_t addr_len = sizeof(struct sockaddr);
	toBytes(msg,c);
	sendto(sockfd, c, MAXMSGLEN, 0,(struct sockaddr *)&reciever_addr, sizeof(struct sockaddr));
	int numbBytesRecieved=recvfrom(sockfd, recieve_buffer, MAXMSGLEN , 0,(struct sockaddr *)&reciever_addr, &addr_len);
	struct message* msg22= toMessage(recieve_buffer);
	close(sockfd);
	return msg22;
}


pthread_mutex_t mtx;
char* server_hostname;
unsigned short int port_num;//server port number. our port number is auto generated
int client_sockfd;
string data_to_send="";
unsigned int my_client_id;
bool connected_to_server=true;

bool getFileName(char input[]){
    cout<<"\nPlease Enter a Filename: ";
    memset(input,'\0',MAX_FILENAME_LEN);//set all array to zero before reading the user input
    cin.getline(input,MAX_FILENAME_LEN);
    //put a \0 at the end
    input[MAX_FILENAME_LEN-1]='\0';//last char forcefully \0 

}

int getFileMode(char input[]){
    int type22=0;
    while(type22!=READ_MODE && type22!=WRITE_MODE){
	    cout<<"-----------------MODE--------------------\n\t1- Read Mode.\n\t2- Write Mode.\n---------------------------------------------\nMode: ";
	    memset(input,'\0',MAX_FILENAME_LEN);//set all array to zero before reading the user input
	    cin.getline(input,MAX_FILENAME_LEN);
	    //put a \0 at the end
	    input[MAX_FILENAME_LEN-1]='\0';//last char forcefully \0 
	    type22 = atoi(input);
    }
    return type22;
}

int getWriteBufferLength(char input[]){
    int type22=0;
    while(type22==0){
	    cout<<"Enter the write buffer length: ";
	    memset(input,'\0',MAX_FILENAME_LEN);//set all array to zero before reading the user input
	    cin.getline(input,MAX_FILENAME_LEN);
	    //put a \0 at the end
	    input[MAX_FILENAME_LEN-1]='\0';//last char forcefully \0 
	    type22 = atoi(input);
    }
    return type22;
}

int getConnectionMode(char input[]){
    int type22=0;
    while(type22!=BLOCKING && type22!=NON_BLOCKING && type22!= DISCONNECTED){
	    cout<<"-----------------MODE--------------------\n\t1- Blocking.\n\t2- Non Blocking.\n\t3- Disconnected.\n---------------------------------------------\nMode: ";
	    memset(input,'\0',MAX_FILENAME_LEN);//set all array to zero before reading the user input
	    cin.getline(input,MAX_FILENAME_LEN);
	    //put a \0 at the end
	    input[MAX_FILENAME_LEN-1]='\0';//last char forcefully \0 
	    type22 = atoi(input);
    }
    return type22;
}

void* processMessage(void* msg_and_sender2){
	struct message_and_sender* msg_and_sender=(struct message_and_sender*) msg_and_sender2;
	cout<<"Message from "<<msg_and_sender->senderAddress<<","<<msg_and_sender->senderPort<<" recieved.\n";
	//print_message_function((void*) msg_and_sender->msg);
	struct message m1;
	if (msg_and_sender->msg->Command==CREATE_FILE){
		cout<<"Creating File: "<<msg_and_sender->msg->File_Name<<endl;
		if(createFile(msg_and_sender->msg->File_Name)==1){
			cout<<"File Created!"<<endl;
		}
		else{
			cout<<"File Creation Failed!"<<endl;			
		}
	}
	else if (msg_and_sender->msg->Command==WRITE_FILE){
		cout<<"Updating File: "<<msg_and_sender->msg->File_Name<<endl;
		saveFile(msg_and_sender->msg->File_Name,msg_and_sender->msg->File_Data);
	}
	free(msg_and_sender->msg);
	free(msg_and_sender);
	return (void *)0;
}

void* messageListenerThread(void* my_null_ptr){
	char recieve_buffer[MAXBUFLEN];
	pthread_t thread1;
	struct message_and_sender* msg_and_sender=(struct message_and_sender*)NULL;
	unsigned int numbBytesRecieved;
	struct sockaddr_in client_addr;//this is the server_addr btw
	socklen_t addr_len = sizeof(struct sockaddr);
	while(connected_to_server){
		memset(recieve_buffer,'\0', MAXBUFLEN);
		if ((numbBytesRecieved=recvfrom(client_sockfd, recieve_buffer, MAXMSGLEN , 0,(struct sockaddr *)&client_addr, &addr_len)) == -1) {
			perror("Could not recieve incoming message:");
		}
		recieve_buffer[numbBytesRecieved]='\0';
		msg_and_sender= new message_and_sender();
		msg_and_sender->msg=toMessage(recieve_buffer);
		msg_and_sender->senderAddress=inet_ntoa(client_addr.sin_addr);
		msg_and_sender->senderPort=ntohs(client_addr.sin_port);
		pthread_create(&thread1,NULL,processMessage,(void*)msg_and_sender);
	}
}

bool isAlreadyOpen(char fileName[]){
	for (vector<openned_file>::iterator iter = my_openned_files.begin(); iter != my_openned_files.end(); ++iter){
		//see if file is already open
		if(strcmp(fileName,iter->name)==0){
			cout<<"File already open locally."<<endl;
			if(iter->filemode==READ_MODE)
				cout<<"File Mode: Read."<<endl;
			if(iter->filemode==WRITE_MODE)
				cout<<"File Mode: Write."<<endl;
			return true;
		}
	}
	return false;
}

void setToDisconnectedMode(char fileName[], int mode){
	for (vector<openned_file>::iterator iter = my_openned_files.begin(); iter != my_openned_files.end(); ++iter){
		//see if file is already open
		if(strcmp(fileName,iter->name)==0){
			iter->connectionmode=mode;
		}
	}
}


bool getToDisconnectedMode(char fileName[]){
	for (vector<openned_file>::iterator iter = my_openned_files.begin(); iter != my_openned_files.end(); ++iter){
		//see if file is already open
		if(strcmp(fileName,iter->name)==0 && iter->connectionmode==DISCONNECTED){
			return true;
		}
	}
	return false;
}


bool isAlreadyOpenForWrite(char fileName[]){
	for (vector<openned_file>::iterator iter = my_openned_files.begin(); iter != my_openned_files.end(); ++iter){
		//see if file is already open
		if(strcmp(fileName,iter->name)==0 && iter->filemode==WRITE_MODE){
			cout<<"File already open locally."<<endl;
			if(iter->filemode==READ_MODE)
				cout<<"File Mode: Read."<<endl;
			if(iter->filemode==WRITE_MODE)
				cout<<"File Mode: Write."<<endl;
			return true;
		}
	}
	return false;
}

void removeMyFile(char* fileName){
	for (vector<openned_file>::iterator cli_it = my_openned_files.begin(); cli_it != my_openned_files.end();)
	{
		if(strcmp(fileName,cli_it->name)==0){
			cli_it=my_openned_files.erase(cli_it);
		}
		else{
			 ++cli_it;
		}
	}
}

void closeThis(char* fileName, char* serverAddress, unsigned short int serverPort){
	ifstream myfile;
	if(!isAlreadyOpen(fileName)){
	cout<<"The file is not open and hence can't be closed!"<<endl;
	return;
	}
	struct message* m1=(struct message *) malloc(sizeof(struct message));
	m1->Magic = MAGIC;
	m1->Client_ID = 0;
	m1->Command = JOIN_TO_SERVER;
	m1->return_port = 0;
	m1->confirmation = 0;
	m1->File_Mode=0;
	m1->Query_Type = 0;
	m1->File_Name_Length=0;
	strcpy(m1->File_Name, "");
	strcpy(m1->File_Data, "");
	m1->File_Data_Length=0;

	if(isAlreadyOpenForWrite(fileName)){
		if(getToDisconnectedMode(fileName)){
			//tell server to write this file!
			m1->Magic = MAGIC;
			m1->confirmation = 0;
			m1->File_Mode=WRITE_MODE;
			m1->Client_ID = my_client_id;
			m1->Command = WRITE_FILE;
			m1->confirmation = 0;
			m1->Query_Type = BLOCKING;
			int begin,end;
			myfile.open(fileName);
			begin = (int)myfile.tellg();
			myfile.seekg (0, ios::end);
			end = (int)myfile.tellg();
			myfile.close();
	      ///////////////////////
            myfile.open(fileName);
			int mybuflen;//we use a dynamic length since i think tcp waits for the whole packet to fill up
	        mybuflen=min(end-begin,MAX_FILESIZE_LIMIT);
	        myfile.read(m1->File_Data,mybuflen);
	        myfile.close();
			m1->File_Data_Length=mybuflen;
			strcpy(m1->File_Name,fileName);
			m1->File_Name_Length=strlen(m1->File_Name);
			sendMessageAndRecieveReply(serverAddress,serverPort,m1);
		}
	}
	//now tell server to close the file
	m1->Magic = MAGIC;
	m1->confirmation = 0;
	m1->File_Mode=0;
	m1->Client_ID = my_client_id;
	m1->Command = CLOSE_FILE;
	m1->confirmation = 0;
	m1->Query_Type = 0;
	strcpy(m1->File_Data, "");
	m1->File_Data_Length=0;
	strcpy(m1->File_Name,fileName);
	m1->File_Name_Length=strlen(m1->File_Name);
	sendMessageAndRecieveReply(serverAddress,serverPort,m1);

	//now delete localcopy.
	removeMyFile(fileName);
	deleteFile(fileName);
	free(m1);
}

int main(int argc, char *argv[])
{	
	printf("Running Client...\n");
	unsigned short int local_port;
	unsigned int query_type;
	char* port_num_char;
	pthread_t thread1;
	ifstream myfile;
	struct hostent* server_ent;
	struct sockaddr_in server_addr;
	struct sockaddr_in req_client_addr;
	struct sockaddr_in msg_sender_addr;
	socklen_t addr_len;
	int numbBytesRecieved;
	int numbBytesSent;
	struct message *m1;
	char recieve_buffer[MAXBUFLEN];
	char msg_to_send_array[MAXMSGLEN];
	pthread_t worker_thread;
	struct addrinfo server_hints,*server_info,*p;
	pthread_mutex_init(&mtx,NULL);
	char* serverAddress;
	unsigned short int serverPort;
	m1 = (struct message *)NULL;
	/*
     * check command line arguments
     */
    //check server
    string file_name(argv[0]);
     if(argc < 3){
		perror("Arguments");
		printAndExit(file_name);
	}
	if((server_ent=gethostbyname(argv[1])) == NULL){
		herror("gethostbyname");
		printAndExit(file_name);
	}
	port_num_char=argv[2];
	port_num=(short)(atoi(port_num_char));
	if(port_num> 65535){
		perror("Port Number");
		printAndExit(file_name);
	}
	//creating socket
	if ((client_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		printAndExit(file_name);
	}
	struct timeval tv;

	memset(&server_hints,0,sizeof(server_hints));
	server_hints.ai_family=AF_INET;//ipv4
	server_hints.ai_socktype=SOCK_DGRAM;
	int addrinfovalue;
	if((addrinfovalue = getaddrinfo(argv[1], port_num_char, &server_hints, &server_info)) != 0){
		perror("Server Not Found");
		printAndExit(file_name);
	}

	m1 = (struct message *) malloc(sizeof(struct message));
	
	m1->Magic = MAGIC;
	m1->Client_ID = 0;
	m1->Command = JOIN_TO_SERVER;
	m1->return_port = 0;
	m1->confirmation = 0;
	m1->File_Mode=0;
	m1->Query_Type = 0;
	m1->File_Name_Length=0;
	strcpy(m1->File_Name, "");
	strcpy(m1->File_Data, "");
	m1->File_Data_Length=0;

	//send msg to server and wait for response
	server_addr.sin_family=AF_INET;
	server_addr.sin_port=htons((short)port_num);
	server_addr.sin_addr=*((struct in_addr *)server_ent->h_addr);
	memset(&(server_addr.sin_zero), '\0', 8); // zero the rest of the struct

	//bind to a local port
	req_client_addr.sin_family=AF_INET;
	req_client_addr.sin_addr.s_addr=htonl(INADDR_ANY);//my address
	req_client_addr.sin_port=0;//random local port
	bind(client_sockfd,(struct sockaddr*)& req_client_addr,sizeof(req_client_addr));
	unsigned int sockaddr_size_client=sizeof(req_client_addr);
	if(getsockname(client_sockfd,(struct sockaddr*)&req_client_addr,&(sockaddr_size_client))!=0){
		close(client_sockfd);
		perror("getsockname");
		printAndExit(argv[0]);
	}
	local_port=ntohs(req_client_addr.sin_port);
//	cout<<"The local IP address is: "<<inet_ntoa(req_client_addr.sin_addr)<<endl;
	cout<<"The local port bound for connection: "<<local_port<<endl;
	addr_len = sizeof(struct sockaddr);

	//request and connect the server
	struct message_and_sender* msg_and_sender=(struct message_and_sender*)NULL;
	toBytes(m1,msg_to_send_array);
	while ((numbBytesSent = sendto(client_sockfd, msg_to_send_array, MAXMSGLEN, 0,
			(struct sockaddr *)&server_addr, sizeof(struct sockaddr))) == -1) {
		cout<<"Send Error! Retrying."<<endl;
	}
	serverAddress= inet_ntoa(server_addr.sin_addr);
	serverPort=ntohs(server_addr.sin_port);
	//recieve the okay message.
	memset(recieve_buffer,'\0', MAXBUFLEN);
	if ((numbBytesRecieved=recvfrom(client_sockfd, recieve_buffer, MAXMSGLEN , 0,(struct sockaddr *)&msg_sender_addr, &addr_len)) == -1) {
		cout<<"No Connection To the Server"<<endl;
		exit(-1);
	}
	recieve_buffer[numbBytesRecieved]='\0';
	m1=toMessage(recieve_buffer);
	cout<<"Connected to the Server and recieved Client ID: "<<m1->Client_ID<<endl;
	my_client_id=m1->Client_ID;
	connected_to_server=true;
	int inputLen=MAX_FILENAME_LEN;//not using string
	char input[inputLen];
	char fileName[inputLen];
	int fileMode=0;
	int connectionMode=0;
	int inputCommand;
	char* fileWriteBuffer;
	int fileWriteBufferLen;
	struct message* m2;
	int myMsgConfirmation=0;
	//you wanna start any thread? Start that thread here
	pthread_create(&thread1,NULL,messageListenerThread,(void*)NULL);
	//threads started
	ifstream fin;
	string finReadBuffer;
	ofstream fout;

	while(connected_to_server){
	    cout<<"-----------------MENU--------------------\n\t1- Create a File.\n\t2- Open a File.\n\t3- Read File.\n\t4- Write File.\n\t5- Close File.\n\t6- Quit.\n---------------------------------------------\nCommand: ";
	    memset(input,'\0',inputLen);//set all array to zero before reading the user input
	    cin.getline(input,inputLen);
	    //put a \0 at the end
	    input[inputLen-1]='\0';//last char forcefully \0 
	    inputCommand=atoi(input);
	    fileMode=0;
	    connectionMode=0;
		m1->Client_ID = my_client_id;
		strcpy(m1->File_Data, "");
		m1->File_Data_Length=0;

	    memset(fileName,'\0',inputLen);
	    if(inputCommand==NO_COMMAND){
	    	cout<<"Sorry! Invalid Command.\n";
	    	continue;
	    }
	    else if(inputCommand==CREATE_FILE){
	    	getFileName(input);
			strcpy(fileName,input);
			//ask server to create the file.
			m1->Client_ID = my_client_id;
			m1->Command = CREATE_FILE;
			m1->confirmation = 0;
			m1->Query_Type = 0;
			strcpy(m1->File_Data, "");
			m1->File_Data_Length=0;
			strcpy(m1->File_Name,fileName);
			m1->File_Name_Length=strlen(m1->File_Name);
			m2 = sendMessageAndRecieveReply(serverAddress,serverPort,m1);
			myMsgConfirmation=m2->confirmation;
			free(m2);
			if(myMsgConfirmation==-1){
				cout<<"File Not Created! Server Error in creating a new file! It may already exist."<<endl;
				continue;
			}
			cout<<"File Created!"<<endl;
	    }
	    else if(inputCommand==OPEN_FILE){
	    	getFileName(input);
			strcpy(fileName,input);
			fileMode=getFileMode(input);
			//see if file is already open, locally!
			if(isAlreadyOpen(fileName)){
				continue;
			}
			//ask server to get file. go for read and write mode.
			m1->Magic = MAGIC;
			m1->confirmation = 0;
			m1->File_Mode=fileMode;
			m1->Client_ID = my_client_id;
			m1->Command = OPEN_FILE;
			m1->confirmation = 0;
			m1->Query_Type = 0;
			strcpy(m1->File_Data, "");
			m1->File_Data_Length=0;
			strcpy(m1->File_Name,fileName);
			m1->File_Name_Length=strlen(m1->File_Name);
			m2 = sendMessageAndRecieveReply(serverAddress,serverPort,m1);
			//now see if it was read or write mode.
			if(m2->confirmation==-1){
				cout<<"File doesn't exist on server."<<endl;
				free(m2);
				continue;
			}
			if(fileMode==WRITE_MODE){
				if(m2->confirmation==-2){
					cout<<"Can't open a file in write mode. It is already open."<<endl;
					free(m2);
					continue;
				}
			}
			createFile(fileName);
			saveFile(fileName,m2->File_Data);
			//add file to my list.
			struct openned_file new_openned_file;
			strcpy(new_openned_file.name,fileName);
			new_openned_file.filemode=fileMode;//read or write
			new_openned_file.connectionmode=0;
    		new_openned_file.client_id=my_client_id;
    		my_openned_files.push_back(new_openned_file);
			free(m2);
	    }
	    else if(inputCommand==READ_FILE){
	    	getFileName(input);
			strcpy(fileName,input);
			//now see if the file is open for read.
			//now read it.	    	
			//see if file exists!
			fin.open(fileName);
		    if (!fin.good()) {
		        fin.close();
		        cout<<"File doesn't exist!"<<endl;
		        continue;
		    } 
			if(isAlreadyOpen(fileName)){
				cout<<"The contents of "<<fileName<<":\n\n";
				while(!fin.eof())
				{
					getline(fin,finReadBuffer);//for a tab
					cout<<finReadBuffer<<endl;
				}
				fin.close();
				cout<<"\n\n";
			}
			else{
				cout<<"Sorry! File is not open.\n";
				fin.close();
			}
	    }
	    else if(inputCommand==WRITE_FILE){
	    	getFileName(input);
			strcpy(fileName,input);
			connectionMode=getConnectionMode(input);
			fileWriteBufferLen=getWriteBufferLength(input);

			if(fileWriteBufferLen<2){fileWriteBufferLen=2;}
			if(fileWriteBufferLen>16384){cout<<"Too big a file.\n";continue;}
			
			fileWriteBuffer=new char[fileWriteBufferLen];
		    memset(fileWriteBuffer,'\0',fileWriteBufferLen);//set all array to zero before reading the user input
		    cout<<"Enter the contents:\n";
		    cin.getline(fileWriteBuffer,fileWriteBufferLen);
		    //force null termination.
		    fileWriteBuffer[fileWriteBufferLen-1]=0;
		    //now write to the file!
		    if(isAlreadyOpenForWrite(fileName)){
	    		setToDisconnectedMode(fileName,connectionMode);
		    	if(connectionMode==DISCONNECTED){
		    		saveFile(fileName,fileWriteBuffer);
		    	}
		    	else{
		    		saveFile(fileName,fileWriteBuffer);
					m1->Magic = MAGIC;
					m1->confirmation = 0;
					m1->File_Mode=WRITE_MODE;
					m1->Client_ID = my_client_id;
					m1->Command = WRITE_FILE;
					m1->confirmation = 0;
					m1->Query_Type = connectionMode;
					strcpy(m1->File_Data, fileWriteBuffer);
					m1->File_Data_Length=strlen(m1->File_Data);
					strcpy(m1->File_Name,fileName);
					m1->File_Name_Length=strlen(m1->File_Name);
					m2 = sendMessageAndRecieveReply(serverAddress,serverPort,m1);
					free(m2);
		    	}
		    }
			else{
				cout<<"Sorry! File is not open in write mode.\n";
				fin.close();
			}

			delete[] fileWriteBuffer;
	    }
	    else if(inputCommand==CLOSE_FILE){
	    	getFileName(input);
			strcpy(fileName,input);
	    	//check if the file is open or not.
	    	closeThis(fileName,serverAddress,serverPort);
	    }
	    else if(inputCommand==QUIT_SYSTEM){
	    	//close all the files.
	    	//close all the sockets.
	    	vector<string> myfilenames;
			for (vector<openned_file>::iterator iter = my_openned_files.begin(); iter != my_openned_files.end(); ++iter){
				cout<<iter->name<<endl;
				myfilenames.push_back(iter->name);
			}
			for(vector<string>::iterator iter = myfilenames.begin(); iter != myfilenames.end(); ++iter){
		    	closeThis((char*)(*iter).c_str(),serverAddress,serverPort);
			}
	   		connected_to_server=false;
	    }
	    else {
	    	cout<<"Sorry! Invalid Command.\n";
	    	continue;
	    }

	}

	/* Deallocating memmory m1, m2 */
	free(m1);
	close(client_sockfd);
	return 1;
}

