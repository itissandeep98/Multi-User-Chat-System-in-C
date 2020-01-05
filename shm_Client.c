
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<signal.h>

key_t key;
key_t key1;
key_t key2;

int shmid ;
int shmid1 ;
int shmid2 ;


char *data;
char *send_memory;
char *receive_memory;
char other[100];
char name[20];
int cond=1;

pthread_t thread;


void sigintHandler(int sig_num){
	fflush(stdout);
	printf("!!!!exiting!!!!\n");
	cond=0;
	shmdt(data);
	// shmdt(send_memory);
	shmctl(shmid,IPC_RMID,NULL);
	shmctl(shmid1,IPC_RMID,NULL);
	pthread_join(thread,NULL);
	exit(0);
}

void *receive(){
	strncpy(other,receive_memory,100);
	while(cond){
        if(strcmp(other,receive_memory)==0){
            continue;
        }
		fflush(stdout);
	    printf("Data RECEIVED: %s\n",receive_memory);
        strncpy(other,receive_memory,100);
    }
}



int main(){
	signal(SIGINT, sigintHandler);
	key=1000;
	shmid = shmget(key,1024,0666|IPC_CREAT);
	data = (char*) shmat(shmid,(void*)0,0);

	key1 = getpid();
	shmid1 = shmget(key1,1024,0666|IPC_CREAT);
	send_memory = (char*) shmat(shmid1,(void*)0,0);

	key2 = key1+1;
	shmid2 = shmget(key2,1024,0666|IPC_CREAT);
	receive_memory = (char*) shmat(shmid2,(void*)0,0);

	fflush(stdout);
	printf("Enter Your name: ");
	fgets(name,20,stdin);
	name[strlen(name)]=' ';
    name[strcspn(name,"\n")]=0;

	sprintf(data,"%s %d",name,key1);
	fflush(stdout);
	printf("Successful Connection %s \n",data);


	pthread_create(&thread, NULL, receive, NULL );

	while (1){
		fgets(send_memory,100,stdin);
	}

	return 0;
}
