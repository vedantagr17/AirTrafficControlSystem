#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <errno.h>
#include <sys/msg.h>
#include <pthread.h>

#define MAXLEN 512
pthread_mutex_t mutex[11];

struct msg_t{
  long mtype;
  int msg_text[MAXLEN];  
};
void *depart(void* arg){
    //printf("In depart");
    int *runway_loads = (int*)arg;
    int msgqid;
    key_t msgkey;
    msgkey = ftok("plane.c", 'A');
    msgqid = msgget(msgkey, IPC_CREAT | 0666);
    if (msgqid == -1) {
        perror("Failed to create message queue");
        exit(1);
    }
    int runway_count = runway_loads[10];
    int weight = runway_loads[11];
    int plane_id = runway_loads[12];
    int airport_id = runway_loads[13];
    int arrival_airport = runway_loads[14];
    int runway_used = runway_count;
    int weight_assigned = 15000;
    for(int i = 0; i<runway_count; i++){
        if(runway_loads[i] >= weight && weight_assigned>runway_loads[i]){
            weight_assigned = runway_loads[i];
            runway_used = i;
        }
    }
    pthread_mutex_lock(&mutex[runway_used]);
    sleep(5);//boarding time
    struct msg_t message;
    message.mtype = 21;
    message.msg_text[0] = airport_id + 10;
    message.msg_text[1] = 1;  //departing status
    message.msg_text[2] = arrival_airport;
    message.msg_text[3] = plane_id;
    message.msg_text[4] = weight;
    if (msgsnd(msgqid, &message,MAXLEN,IPC_NOWAIT) == -1){
    	printf("error in sending message\n");
        exit(1);
    }
    printf("\nPlane %d has completed boarding/loading and taken off from Runway No. %d of Airport No. %d.\n", plane_id, runway_used+1, airport_id);
    pthread_mutex_unlock(&mutex[runway_used]);

}
void *arrive(void* arg){
    int* runway_loads = (int*) arg;
    sleep(30);//travel time
    int msgqid;
    key_t msgkey;
    msgkey = ftok("plane.c", 'A');
    msgqid = msgget(msgkey, IPC_CREAT | 0666);
    if (msgqid == -1) {
        perror("Failed to create message queue");
        exit(1);
    }
    int runway_count = runway_loads[10];
    int weight = runway_loads[11];
    int plane_id = runway_loads[12];
    int airport_id = runway_loads[13];
    int runway_used = runway_count;
    int weight_assigned = 15000;
    for(int i = 0; i<runway_count; i++){
        if(runway_loads[i] >= weight && weight_assigned>runway_loads[i]){
            weight_assigned = runway_loads[i];
            runway_used = i;
        }
    }
    pthread_mutex_lock(&mutex[runway_used]);
    sleep(5);//unloading time
    struct msg_t message;
    message.mtype = 21;
    message.msg_text[0] = airport_id + 10;
    message.msg_text[1] = 0;  //departing status
    message.msg_text[2] = runway_loads[14]; // departed from
    message.msg_text[3] = plane_id;
    if (msgsnd(msgqid, &message,MAXLEN,IPC_NOWAIT) == -1){
    	  printf("error in sending message\n");
        exit(1);
    }
    printf("\nPlane %d has landed on Runway No. %d of Airport No. %d and has completed deboarding/unloading.\n", plane_id, runway_used+1, airport_id);
    
    pthread_mutex_unlock(&mutex[runway_used]);

}

int main(){
    int airport_id;
    int msgqid;
    int thread_count=0;
    key_t msgkey;
    pthread_t tid[10];
	pthread_attr_t attr;
    struct msqid_ds buf;
    int argum[20][15];
    int arg_counter = 0;
    pthread_attr_init(&attr); 
    printf("Enter Airport Number: ");
    if (scanf("%d", &airport_id) != 1 || airport_id < 1 || airport_id > 10) {
        fprintf(stderr, "Invalid airport number (must be between 1 and 10 (inclusive))\n");
        exit(1);
    }
    msgkey = ftok("plane.c", 'A');
    msgqid = msgget(msgkey, IPC_CREAT | 0666);
    if (msgqid == -1) {
        perror("Failed to create message queue");
        exit(1);
    }
    int runway_count;
    printf("Runway count: ");
    if (scanf("%d", &runway_count) != 1 || runway_count < 1 || runway_count > 10) {
        fprintf(stderr, "Invalid runway number (must be between 1 and 10 (inclusive))\n");
        exit(1);
    }
    for(int i=0; i<runway_count+1; i++){
        int x = pthread_mutex_init(&mutex[i], NULL);
        if(x){
            printf("Failed to create pthread mutex error id: %d", x);
            exit(1);
        }
    }
    int runway_loads[15];
    //printf("In departure before while llop1");
    runway_loads[10] = runway_count;
    //printf("In departure before while llop2");
    for(int i = 0;i<runway_count;i++){
        printf("Enter weight of runway %d: \n", i+1);
        scanf("%d", &runway_loads[i]);
    }
    //printf("In departure before while llop");
    struct msg_t message;
    int count = 0;
    while(10>9){
       //printf("In While loop");
        int rc = msgrcv(msgqid, (void *)&message, MAXLEN, airport_id + 10, IPC_NOWAIT);
        if (rc == -1) {
            if (errno == ENOMSG) {
                printf("\rWaiting");
                fflush(stdout);
                sleep(1);
                printf(".");
                fflush(stdout);
                sleep(1);
                printf(".");
                fflush(stdout);
                sleep(1);
                printf(".");
                fflush(stdout);
                printf("\r");
                printf("\033[K");
            } else {
                perror("msgrcv error: ");
                break; // Exit the program
            }
        } else {
            int type = message.msg_text[2];
            if(type == 1){ // departure
                runway_loads[11] = message.msg_text[1]; //weight
                runway_loads[12] = message.msg_text[0]; // plane_id
                //printf("plane %d about to depart\n", runway_loads[12]);
                runway_loads[13] = airport_id;
                runway_loads[14] = message.msg_text[3]; // arrival airport number
                //printf("In departure before thread");
                for(int i = 0;i<15;i++){
                    argum[arg_counter%20][i] = runway_loads[i];
                }
                pthread_create(&tid[thread_count++%10], &attr, depart, &argum[arg_counter%20]);
                arg_counter++;
            }else if(type == 2){ // arrival
                runway_loads[11] = message.msg_text[1]; //weight of plane
                runway_loads[12] = message.msg_text[0]; //plane id
                runway_loads[13] = airport_id;  //arrived at
                runway_loads[14] = message.msg_text[3]; //departed from
                for(int i = 0;i<15;i++){
                    argum[arg_counter%20][i] = runway_loads[i];
                }
                pthread_create(&tid[thread_count++%10], &attr, arrive, &argum[arg_counter%20]);
                arg_counter++;
            }  else if(type == -10) {
                for(int i =0; i<runway_count+1; i++){
                    pthread_mutex_lock(&mutex[i]);
                }
                for(int i=0; i<runway_count+1; i++){
                    pthread_mutex_unlock(&mutex[i]);
                    int x = pthread_mutex_destroy(&mutex[i]);
                    if(x != 0){
                        printf("Failed to destroy pthread mutex error id: %d", x);
                    }
                }
                if (msgctl(msgqid, IPC_STAT, &buf) == -1) {
                    perror("msgctl");
                    exit(EXIT_FAILURE);
                }
                if(buf.msg_lspid == 1){
                    if (msgctl(msgqid, IPC_RMID, NULL) == -1) {
                        perror("msgctl: ");
                        exit(EXIT_FAILURE);
                    }
                }
                break;
            }
        }
    }
    return 0;    
}