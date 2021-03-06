#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<errno.h>
#include<pthread.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/shm.h>
#include<sys/ipc.h>
#include<sys/select.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include"logger.h"
#include<string.h>
#include<signal.h>
#include"JSON.h"
#include"JSONValue.h"
#include"json_handler.h"
#include"DataBlock.h"

//#define DEBUG
#define MAX_CONNECTIONS 10
#define SEND_BUFFER_SIZE 200	//bytes
#define RECV_BUFFER_SIZE 500	//bytes

using namespace std;
//Add JSON functionality to parse and send messages to minions.

//Global variables for program
int success_buf_flag = 1;
char log_message[100];
int log_to_terminal = 0;
int bytes_read[MAX_CONNECTIONS];	
int send_ret[MAX_CONNECTIONS];
void *shared_mem_vicon;
void *shared_mem_comm;
char success_buf[SEND_BUFFER_SIZE] = "\{ \"Backward\":-1.0,\"Forward\":-1.0,\"ID\":109,\"Left\":-1.0,\"Right\":-1.0,\"Stop\":1.0\}";
/*char stop_buf[SEND_BUFFER_SIZE] = "\{\\\"Backward\\\":-1.0,\\\"Forward\\\":-1.0,\\\"ID\\\":109,\\\"Left\\\":-1.0,\\\"Right\\\":-1.0,\\\"Stop\\\":1.0\}";
char forward_buf[SEND_BUFFER_SIZE] =  "\{\\\"Backward\\\":-1.0,\\\"Forward\\\":0.5,\\\"ID\\\":109,\\\"Left\\\":-1.0,\\\"Right\\\":-1.0,\\\"Stop\\\":-1.0\}";
char right_buf[SEND_BUFFER_SIZE] = "\{\\\"Backward\\\":-1.0,\\\"Forward\\\":-1.0,\\\"ID\\\":109,\\\"Left\\\":-1.0,\\\"Right\\\":0.25,\\\"Stop\\\":-1.0\}";
char left_buf[SEND_BUFFER_SIZE] = "\{\\\"Backward\\\":-1.0,\\\"Forward\\\":-1.0,\\\"ID\\\":109,\\\"Left\\\":0.5,\\\"Right\\\":-1.0,\\\"Stop\\\":-1.0\}";
char backward_buf[SEND_BUFFER_SIZE] = "\{\\\"Backward\\\":0.5,\\\"Forward\\\":-1.0,\\\"ID\\\":109,\\\"Left\\\":-1.0,\\\"Right\\\":-1.0,\\\"Stop\\\":-1.0\}";
*/
FILE *log_file = NULL;

//Add -v to printf logger details to std terminal

//Thread definitions
pthread_t bot_thread[MAX_CONNECTIONS];
void server_close();

//Add SIGINT handler to stop server listen thread.
//Allows for resource deallocation
//Signal SIGINT callback

void sigint_callback(int sig) {
		
	sprintf(log_message, "Inside sigint_callback for sig %d\n", sig);
	logger(log_file, "DEBUG", log_message);
	memset(log_message, 0, sizeof(log_message));
	server_close();
	exit(0);

}


//close server connection
void server_close() {

//int i;
shmdt(shared_mem_vicon);
shmdt(shared_mem_comm);
//release resources
/*
for (i = 0 ; i < MAX_CONNECTIONS; i++) {
    pthread_cancel(bot_thread[i]);
    pthread_cancel(bot_thread[i]);
}
*/
fclose(log_file);

}

void *bot_control_thread(void *arg) {

	int client_fd = *(int *)arg;
	int action = 0;	
	char id[10] = "m3pi1";

	while(1) {
		if(send_ret[client_fd] <= 0) {
			sleep(1);
			continue;
		} else {
			switch(action) {

				case 0: 
					json_create(success_buf, id, 0.5, -1.0, -1.0, -1.0, -1.0);
					break;
				case 1: 	
					json_create(success_buf, id, 0.5, -1.0, -1.0, -1.0, -1.0);
					break;	
				case 2:
					json_create(success_buf, id, -1.0, -1.0, -1.0, 0.25, -1.0);
					break;
				case 3:	
					json_create(success_buf, id, 0.5, -1.0, -1.0, -1.0, -1.0);
					break;
				case 4:	
					json_create(success_buf, id, -1.0, -1.0, -1.0, -1.0, 1.0);
					break;
				case 5:
					json_create(success_buf, id, -1.0, 0.5, -1.0, -1.0, -1.0);
					break;	
				case 6:	
					json_create(success_buf, id, -1.0, -1.0, -1.0, 0.25, -1.0);
					break;
				case 7:	
					json_create(success_buf, id, 0.5, -1.0, -1.0, -1.0, -1.0);
					break;	
				case 8:	
					json_create(success_buf, id, 0.5, -1.0, -1.0, -1.0, -1.0);
					break;	
				default: 
					json_create(success_buf, id, -1.0, -1.0, -1.0, -1.0, 1.0);
					break;
			}	
			action++;
			success_buf_flag = 1;
			sleep(1);
		}

	}	

}

//main function

int main() {

int shmid;
int shmid2;
int action = 0;
struct TrackerObject *bot_location;
char *json_buf;

  shmid = shmget((key_t)1234, sizeof(struct TrackerObject), 0666 | IPC_CREAT);

  if(shmid < 0) {
	  perror("msgget failed with error");
	  exit(0);
  }
  
  shmid2 = shmget((key_t)4321, SEND_BUFFER_SIZE, 0666 | IPC_CREAT);

  if(shmid2 < 0) {
	  perror("msgget failed with error");
	  exit(0);
  }

shared_mem_vicon = shmat(shmid, (void *)0 , 0);
if (shared_mem_vicon == (void *)-1) {
	printf("shmat for shared_mem_vicon failed\n");
	exit(0);
}

shared_mem_comm = shmat(shmid2, (void *)0 , 0);
if (shared_mem_comm == (void *)-1) {
	printf("shmat for shared_mem_comm failed\n");
	exit(0);
}
bot_location = (struct TrackerObject *)shared_mem_vicon;
json_buf = (char *)shared_mem_comm;
char *id = "m3pi";

//log_file = fopen("/home/ubuntu/experiment_space/server_program/src/log.txt", "w+");
//if(log_file == NULL) {
	log_file = stdout;
//}

signal(SIGINT, &sigint_callback);

//Maintain a while 1 loop, acting as server listening/accept thread

/*
		ret = pthread_create(&bot_thread2[2], NULL, &bot_control_thread, &client_fd);
		if(ret < 0) {
			perror("Thread creation failed __LINE__\n");
		}
pthread_join();
*/

while(1) {

if(action == 10)
action = 0;
#ifndef DEBUG
	printf("Item Name :%s\n", bot_location->ItemName);
	printf("Trans X :%lf\n", bot_location->TransX);
	printf("Trans Y :%lf\n", bot_location->TransY);
	printf("Trans Z :%lf\n", bot_location->TransZ);
	printf("Rot X :%lf\n", bot_location->RotX);
	printf("Rot Y :%lf\n", bot_location->RotY);
	printf("\n\n");
#endif
	switch(action) {

		case 0: 
			json_create(success_buf, id, 0.5, -1.0, -1.0, -1.0, -1.0);
			break;
		case 1: 	
			json_create(success_buf, id, 0.5, -1.0, -1.0, -1.0, -1.0);
			break;	
		case 2:
			json_create(success_buf, id, -1.0, -1.0, -1.0, 0.25, -1.0);
			break;
		case 3:	
			json_create(success_buf, id, 0.5, -1.0, -1.0, -1.0, -1.0);
			break;
		case 4:	
			json_create(success_buf, id, -1.0, -1.0, -1.0, -1.0, 1.0);
			break;
		case 5:
			json_create(success_buf, id, -1.0, 0.5, -1.0, -1.0, -1.0);
			break;	
		case 6:	
			json_create(success_buf, id, -1.0, -1.0, -1.0, 0.25, -1.0);
			break;
		case 7:	
			json_create(success_buf, id, 0.5, -1.0, -1.0, -1.0, -1.0);
			break;	
		case 8:	
			json_create(success_buf, id, 0.5, -1.0, -1.0, -1.0, -1.0);
			break;	
		default: 
			json_create(success_buf, id, -1.0, -1.0, -1.0, -1.0, 1.0);
			break;
	}	
action++;
/*
	printf("bot_location is %f\n", bot_location->TransX);

	if (bot_location->TransX < 1000.0) {
			json_create(success_buf, id, 0.5, -1.0, -1.0, -1.0, -1.0);
	} else if (bot_location->TransX > 1000.0) {
			json_create(success_buf, id, -1.0, -1.0, -1.0, 0.25, -1.0);	
	} else {
			json_create(success_buf, id, -1.0, -1.0, -1.0, -1.0, 1.0);
	}
*/
memcpy(json_buf, success_buf, sizeof(success_buf));

sleep(1);

}

server_close();

return 1;
}
