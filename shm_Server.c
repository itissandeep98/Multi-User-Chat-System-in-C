#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<ctype.h>
#include <sys/types.h>
#include<signal.h>

key_t key;

int shmid ;

char *clientdata;
char prevClient[100]; //previous client name
char** parsed;

int clientCount = 0;

pthread_t thread[1024];

int cond=1;


struct client{
	char name[20];
	int key;
	int index;
};
struct client Client[1024];

void sigintHandler(int sig_num){
	fflush(stdout);
	printf("exiting\n");
	cond=0;
	shmdt(clientdata);
	shmctl(shmid,IPC_RMID,NULL);
	for(int i = 0 ; i < clientCount ; i ++)
		pthread_join(thread[i],NULL);

	exit(0);
}

char **parse(char* input){
    char** arr = (char**) malloc(sizeof(char*)*100);
    for(int i=0; i<100; ++i){
        arr[i]= (char*) malloc(sizeof(char)*100);
    }
    int i=0,j=0,k=0;
    while(*(input+i) !='\0'){
        if(*(input+i)!=' '){
            arr[j][k]=input[i];
            k+=1;
        }
        else if(*(input+i)==' ' && isspace(*(input+i+1))==0){
            j+=1;
            k=0;
        }
        i+=1;
    }
    arr[j+1]=NULL;
    return arr;
}

void send_to_client(int index,char * msg){
	int shmid_receiver=shmget((key_t)Client[index].key+1,1024,0666|IPC_CREAT);
	char * data_to_send=(char*) shmat(shmid_receiver,(void*)0,0);
	strncpy(data_to_send,msg,100);

	// shmdt(data_to_send);
	// shmctl(shmid_receiver,IPC_RMID,NULL);

}

void * networking(void * ClientDetail){
	struct client* clientDetail = (struct client*) ClientDetail;

	key_t key1=clientDetail->key;
	int shmid1 = shmget(key1,1024,0666|IPC_CREAT);
	char* msg=(char*) shmat(shmid1,(void*)0,0);

	char prevmsg[100];

	strncpy(prevmsg,msg,100);

	while(cond){
        if(strcmp(prevmsg,msg)==0){
            continue;
        }
		fflush(stdout);
	    printf(">>>Data read from Client %d: %s\n",clientDetail->index,msg);
        strncpy(prevmsg,msg,100);

		char** decode_message=parse(msg);

		char * temp=(char *) malloc(20 * sizeof(char));
		if(strcmp(decode_message[0],"SEND")==0 || strcmp(decode_message[0],"send")==0){

			strcpy(temp,decode_message[2]);
			int index=atoi(decode_message[1]);
			if(index<clientCount){
				send_to_client(index,temp);
				fflush(stdout);
				printf(">>>Data Written to client %d : %s\n",index,msg);
			}
		}
		else if(strcmp(decode_message[0],"ALL")==0||strcmp(decode_message[0],"all")==0){
			strcpy(temp,decode_message[1]);
			fflush(stdout);
			printf(">>>Data Sent to every client\n");
			for(int i=0;i<clientCount;i++){
				if(i!=clientDetail->index){
					send_to_client(i,temp);
				}
			}
		}
		else {
			fflush(stdout);
			printf("command Not found\n");
		}


    }
}


int main(){
	fflush(stdout);
	printf("server running\n");
	key = 1000;
	shmid = shmget(key,1024,0666|IPC_CREAT);
	clientdata = (char*) shmat(shmid,(void*)0,0);
	strncpy(clientdata,"nope",100);
	strncpy(prevClient,clientdata,100);
	signal(SIGINT, sigintHandler);

	while(1){
		if(strcmp(prevClient,clientdata)==0){
            continue;
        }
		parsed=parse(clientdata);
		fflush(stdout);
		printf("Connected Client %d %s\n",clientCount,parsed[0]);
		strncpy(prevClient,clientdata,100);


		strcpy(Client[clientCount].name,parsed[0]);
		Client[clientCount].key = atoi(parsed[1]) ;
		Client[clientCount].index = clientCount;
		pthread_create(&thread[clientCount], NULL, networking,  (void *) &Client[clientCount]);
		clientCount++;
	}

	shmdt(clientdata);
	shmctl(shmid,IPC_RMID,NULL);
	for(int i = 0 ; i < clientCount ; i ++)
		pthread_join(thread[i],NULL);

}
