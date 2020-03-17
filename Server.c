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
char prevClient[100]; 
char** parsed;

int ind=0;

int clientCount = 0;
pthread_mutex_t lock;

pthread_t thread[1024];

int cond=1;

typedef struct
{
	char *msglist[20];
} msg;
volatile msg *arr;
struct client{
	char name[20];
	int key;
	int index;
};

struct client Client[100];

void enqueue(char *msg)
{
	ind = ind % 20;
	arr->msglist[ind++] = msg;
}

void show()
{
	int j = 0;
	printf("------------------------------------\n");
	while (j < ind)
	{
		printf("%d: %s\n", j, arr->msglist[j]);
		j++;
	}
	printf("------------------------------------\n");
}
void sigintHandler(int sig_num){
	fflush(stdout);
	printf("exiting\n");
	cond=0;
	shmdt(clientdata);
	shmctl(shmid,IPC_RMID,NULL);
	for(int i = 0 ; i < clientCount ; i ++)
		pthread_join(thread[i],NULL);

	pthread_mutex_destroy(&lock);

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
		pthread_mutex_lock(&lock);
		enqueue(msg);
		

		fflush(stdout);
	    printf(">>>Data read from Client %d: %s\n",clientDetail->index,msg);
        strncpy(prevmsg,msg,100);


		char * temp=(char *) malloc(20 * sizeof(char));
		if(memcmp(msg,"SEND",4)==0 || memcmp(msg,"send",4)==0){
			char *start = &msg[5];
  			char *end = &msg[7];
			char *substr = (char *)calloc(1, end - start + 1);
  			memcpy(substr, start, end - start);
			sprintf(temp,"\t\t%s says %s\n",clientDetail->name, msg+7);
			int index=atoi(substr);
			if( index== clientDetail->index){
				printf("Data can't be written to the same client\n");

			}
			else if(index<clientCount){
				send_to_client(index,temp);
				fflush(stdout);
				printf(">>>Data Written to client %d : %s\n",index,msg);
			}
			else{
				printf("client doesn't exist\n");
			}
		}

		else if(memcmp(msg,"ALL",3)==0||memcmp(msg,"all",3)==0){
			sprintf(temp,"\t\t%s\t: %s\n",clientDetail->name,msg+4);
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
			printf("command Not found: %s\n",msg);
		}
		// sleep(10);
		pthread_mutex_unlock(&lock);
    }
}

void* handleclient(){
	while (cond)
	{
		if (strcmp(prevClient, clientdata) == 0)
		{
			continue;
		}
		parsed = parse(clientdata);
		fflush(stdout);
		printf("Connected Client %d %s\n", clientCount, parsed[0]);
		strncpy(prevClient, clientdata, 100);

		strcpy(Client[clientCount].name, parsed[0]);
		Client[clientCount].key = atoi(parsed[1]);
		Client[clientCount].index = clientCount;
		pthread_create(&thread[clientCount], NULL, networking, (void *)&Client[clientCount]);
		clientCount++;
	}
}



int main(){
	fflush(stdout);
	printf("Server Running\n");

	key = 1000;
	shmid = shmget(key,1024,0666|IPC_CREAT);
	clientdata = (char*) shmat(shmid,(void*)0,0);
	strncpy(clientdata,"nope",100);
	strncpy(prevClient,clientdata,100);
	signal(SIGINT, sigintHandler);

	key_t key3 = 1003;
	int shmid3 = shmget(key3, 1024, 0666 | IPC_CREAT);
	arr = shmat(shmid3, (void *)0, 0);

	if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init has failed\n");
        return 1;
    }
	pthread_t th;
	pthread_create(&th, NULL, handleclient, NULL);

	char *temp = (char *)malloc(sizeof(char) * 100);
	while(1){
		fgets(temp, 100, stdin);
		if (memcmp(temp, "show", 4) == 0 || memcmp(temp, "SHOW", 4) == 0)
		{
			show();
		}
		else if (memcmp(temp, "exit", 4) == 0 || memcmp(temp, "EXIT", 4) == 0){
			sigintHandler(0);
		}
	}
}
