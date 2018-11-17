#ifndef MY_UTIL_C
#define MY_UTIL_C

#include "my_util.h"
#include <stdio.h>
#include <string.h>
#include <fstream>

void toBytes(message *m,char* buf)
{
	memcpy(buf,m,MAXMSGLEN);
}

message* toMessage(char* rcvBuffer)
{
	message* msg = new message();
	memcpy(msg, rcvBuffer, MAXMSGLEN);
	return msg;
}

int createFile(char* fileName){ 
    std::ifstream f(fileName);
    if (f.good()) {
        f.close();
        return -1;
    } 
    f.close();
    //now create a file.
    std::ofstream f2(fileName);
    if(f2.bad()){
    	f2.close();
    	return -1;
    }
    f2<<"";
    f2.close();
    return 1;
}

int saveFile(char* fileName,char mydata[]){ 
    //now create a file.
    std::ofstream f2(fileName);
    if(f2.bad()){
    	f2.close();
    	return -1;
    }
    f2<<mydata;
    f2.close();
    return 1;
}

bool deleteFile(char* fileName){
	if( remove(fileName) != 0 )
    	return false;
  	else
    	return true;
}

#endif
