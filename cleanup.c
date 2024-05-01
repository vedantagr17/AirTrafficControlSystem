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
    int msgqid;
    key_t msgkey;
    struct msg_t message;
    message.mtype = 21;
    message.msg_text[0] = 22;
    msgkey = ftok("plane.c", 'A');
    msgqid = msgget(msgkey, IPC_CREAT | 0666);
    if (msgqid == -1) {
        perror("Failed to create message queue\n");
        exit(1);
    }

    while(1){
        printf("Do you want to cleanup? (Y/N)\n");
        char x;
        scanf("%c", &x); // Added space before %c to consume whitespace characters
        if(x == 'N'){
            sleep(1);
        }
        else if(x == 'Y'){
            //printf("hey\n");
            break;
        } else {
            printf("Please enter (Y/N)...\n");
        }
        sleep(1);
    }
    //printf("outside while\n");
    if (msgsnd(msgqid, &message, MAXLEN, IPC_NOWAIT) == -1){
        perror("Error in sending message\n");
        exit(1);
    }
    printf("Cleanup Request has been sent!\n");
    return 0;
}
