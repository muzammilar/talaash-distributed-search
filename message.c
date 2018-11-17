/* message.c */
#include <stdio.h> 		/* Input/Output */
#include <stdlib.h> 	/* General Utilities */
#include <string.h>
#include <pthread.h>
#include "message.h"

/* Global Variables */


void *print_message_function(void *ptr) {

    struct message *m = (struct message *) ptr;
    printf("-------------------------------------------------\n");
    printf("Magic No : %d\n", m->Magic);
    printf("Client_ID : %d\n", m->Client_ID);
    printf("Command : %d\n", m->Command);
	printf("Return Port : %d\n", m->return_port);
	printf("Confirmation : %d\n", m->confirmation);
	printf("Query_Type : %d\n", m->Query_Type);
    if (m->File_Name_Length > 0) {
        printf("File_Name (%d Bytes) : %s\n", m->File_Name_Length, m->File_Name);
    }
    if (m->File_Data_Length > 0) {
        printf("File_Data (%d Bytes) : %s\n", m->File_Data_Length, m->File_Data);
    }
    printf("-------------------------------------------------\n");
    return (void *)(0);
}