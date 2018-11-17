#ifndef MESSAGE_H
#define MESSAGE_H
/* message.h */

/* Messaging protocol constants and formats */

/* Command definition */

#define NO_COMMAND			0
#define CREATE_FILE			1
#define OPEN_FILE			2
#define READ_FILE			3
#define WRITE_FILE	 		4
#define CLOSE_FILE			5
#define QUIT_SYSTEM			6
#define JOIN_TO_SERVER		7

/* Query Types */
#define BLOCKING		1
#define NON_BLOCKING	2
#define DISCONNECTED	3

/* File Mode */
#define READ_MODE		1
#define WRITE_MODE		2

/* Constants */
#define MAGIC               14582

#define MAX_FILENAME_LEN			30    /*MAX Query length*/
#define MAX_FILESIZE_LIMIT         16384

struct	message {				/* message format of Password cracker protocol	*/
	unsigned int	Magic;
	unsigned int	Client_ID;	
	unsigned int	Command;
	unsigned int 	return_port;
	int 			confirmation;
	unsigned int	Query_Type;
	unsigned int 	File_Mode;
	unsigned int	File_Name_Length;
	char			File_Name[MAX_FILENAME_LEN];	/* Data*/
	unsigned int	File_Data_Length;
	char			File_Data[MAX_FILESIZE_LIMIT];
};
extern void *print_message_function( void *ptr );

#endif
