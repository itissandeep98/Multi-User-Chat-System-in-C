
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
key_t key3;

int shmid ;
int shmid1 ;
int shmid2 ;
int shmid3;


int i = 0;

char *data;
char *send_memory;
char *receive_memory;
char other[100];
char name[20];
int cond=1;

pthread_t thread;

typedef struct
{
	char *msglist[20];
}msg;
volatile msg *arr;
void sigintHandler(int sig_num){
	fflush(stdout);
	printf("!!!!exiting!!!!\n");
	cond=0;
	shmdt(data);
	shmctl(shmid,IPC_RMID,NULL);
	shmctl(shmid1,IPC_RMID,NULL);
	pthread_join(thread,NULL);
	exit(0);
}

void enqueue(char * msg){
	i=i%20;
	arr->msglist[i++]=msg;
}

void show(){
	int j=0;
	printf("------------------------------------\n");
	while(j<i){
		printf("%d: %s\n", j, arr->msglist[j]);
		j++;
	}
	printf("------------------------------------\n");
}

void *receive(){
	strncpy(other,receive_memory,100);
	while(cond){
        if(strcmp(other,receive_memory)==0){
            continue;
        }
		fflush(stdout);
		enqueue(receive_memory);
	    printf("%s",receive_memory);
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

	key3 = 1003;
	shmid3 = shmget(key3, 1024, 0666 | IPC_CREAT);
	arr = shmat(shmid3, (void *)0, 0);

	fflush(stdout);
	printf("Enter Your name: ");
	fgets(name,20,stdin);
	name[strlen(name)]=' ';
    name[strcspn(name,"\n")]=0;

	sprintf(data,"%s %d",name,key1);
	fflush(stdout);
	printf("Successful Connection %s \n",data);


	pthread_create(&thread, NULL, receive, NULL );

	char *temp=(char*)malloc(sizeof(char)*100);

	while (1){
		fgets(temp,100,stdin);
		if (memcmp(temp, "show", 4) == 0 || memcmp(temp, "SHOW", 4) == 0)
		{
			show();
			continue;
		}
		else if (memcmp(temp, "exit", 4) == 0 || memcmp(temp, "EXIT", 4) == 0)
		{
			sprintf(send_memory, "client exiting");
				sigintHandler(0);
		}
		sprintf(send_memory, "%s", temp);
	}

	return 0;
}
