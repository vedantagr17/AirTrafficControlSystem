#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#define MAX_AIRPORTS 10
#define MAXLEN 512

struct msg_t{
  long mtype;
  int msg_text[MAXLEN];  
};

int main() {
  int num_airports;
  int msgqid;
  key_t msgkey;
  FILE *file;
  file = fopen("AirTrafficController.txt", "w");

  printf("Enter the number of airports to be handled/managed: ");
  if (scanf("%d", &num_airports) != 1 || num_airports < 2 || num_airports > 10) {
    fprintf(stderr, "Invalid number of airports (must be between 2 and 10)\n");
    exit(1);
  }

  msgkey = ftok("plane.c", 'A');
  msgqid = msgget(msgkey, IPC_CREAT | 0666);
  if (msgqid == -1) {
    perror("Failed to create message queue");
    exit(1);
  }
  int end = 0;
  struct msg_t message;
  struct msg_t terminate;
  int left = 0,came = 0;
  while(end == 0|| (left != came)){
    int rc = msgrcv(msgqid, (void *)&message, MAXLEN, 21, IPC_NOWAIT);
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
                //printf("\nleft = %d, came = %d\n", left, came);
                sleep(1);
                printf("\r");
                printf("\033[K");
            } else {
                perror("msgrcv error: ");
                break; // Exit the program
            }
        } else{
      int sender = message.msg_text[0];
      if(sender == 22){
        //printf("end = 1\n");
        end = 1;
      }else if(sender >= 1 && sender <= 10){ // plane - need to send message to departing airport
        struct msg_t sending;
        sending.mtype = message.msg_text[2] + 10; // departing airport code
        sending.msg_text[0] = message.msg_text[3]; // plane ID
        sending.msg_text[1] = message.msg_text[4]; // weight
        sending.msg_text[2] = 1; // departing airport indicator
        sending.msg_text[3] = message.msg_text[1]; // arrival airport number
        if (msgsnd(msgqid, &sending,MAXLEN,0) == -1){
          printf("error in sending message\n");
          exit(1);
        }
        //printf("\n");
        //printf("plane %d sent this",message.msg_text[3]);
        //printf("\n");
      }else if(sender >= 11 && sender <= 20){ // airport
        int departing = 0;
        departing = message.msg_text[1];
        if(departing == 1){
          //da -> ATC -> aa
          struct msg_t sending;
          sending.mtype = message.msg_text[2] + 10; // arrival airport code
          sending.msg_text[0] = message.msg_text[3]; // plane ID
          sending.msg_text[1] = message.msg_text[4]; // weight
          sending.msg_text[2] = 2; // arrival airport indicator
          sending.msg_text[3] = message.msg_text[0] - 10; // departure airport id
          if(msgsnd(msgqid, &sending,MAXLEN,0) == -1){
            printf("error in sending message\n");
            exit(1);
          }
          left++;
        }else{
          //aa->ATC->plane
          struct msg_t sending;
          sending.mtype = message.msg_text[3]; // plane code
          sending.msg_text[0] = 1; // terminate plane ID
          sending.msg_text[1] = message.msg_text[0]; //arrived at
          sending.msg_text[2] = message.msg_text[2]; //departed from
          came++;
          if(msgsnd(msgqid, &sending,MAXLEN,IPC_NOWAIT) == -1){
            perror("error in sending message: \n");
            exit(1);
          }
          
          
          if(file == NULL){
            printf("Error opening file");
            exit(1);
          }
          fprintf(file, "Plane %d has departed from Airport %d and will land at Airport %d\n", message.msg_text[3], message.msg_text[2], message.msg_text[0] - 10);
          
        }
      } 
    }
  }
  fclose(file);
  for(int i = 1;i<=num_airports;i++){

    terminate.mtype = i+10;
    terminate.msg_text[2] = -10;
    if (msgsnd(msgqid, &terminate,MAXLEN,IPC_NOWAIT) == -1){
      printf("error in sending message\n");
      exit(1);
    }
  }
  
  return 0;
}
