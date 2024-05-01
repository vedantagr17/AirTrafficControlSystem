#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#define MAXLEN 512

struct msg_t{
  long mtype;
  int msg_text[MAXLEN];  
};

#define MAX_PASSENGERS 10
#define MAX_LUGGAGE_WEIGHT 25
#define MIN_BODY_WEIGHT 10
#define MAX_BODY_WEIGHT 100
#define NUM_CREW_MEMBERS 7
#define AVG_CREW_WEIGHT 75
#define MAX_AIRPORT 10
#define MIN_AIRPORT 1
#define READ_END 0
#define WRITE_END 1

int main() {
    int pfd[MAX_PASSENGERS][2];
    int plane_id, plane_type;
    int num_passengers = 0;
    int total_passenger_weight = 0;
    int crew_weight = NUM_CREW_MEMBERS * AVG_CREW_WEIGHT;
    
    printf("Enter Plane ID: ");
    if (scanf("%d", &plane_id) != 1) {
        printf("Error: Invalid input for plane ID\n");
        return 1;
    }

    printf("Enter Type of Plane (0 for Cargo, 1 for Passenger): ");
    if (scanf("%d", &plane_type) != 1) {
        printf("Error: Invalid input for plane type\n");
        return 1;
    }

    if (plane_type == 1) {
        printf("Enter Number of Passengers: ");
        if (scanf("%d", &num_passengers) != 1 || num_passengers < 0 || num_passengers > MAX_PASSENGERS) {
            printf("Error: Invalid number of passengers\n");
            return 1;
        }
        pid_t passenger_pid[num_passengers];
        int passenger_weights[num_passengers];
        for (int i = 0; i < num_passengers; i++) {
            int luggage_weight, body_weight;
            while (1) {
                printf("Enter Weight of Luggage for Passenger %d: ", i + 1);
                scanf("%d", &luggage_weight);
                if (luggage_weight >= 0 && luggage_weight <= MAX_LUGGAGE_WEIGHT) {
                    break;
                } else {
                    printf("Error: Luggage weight should be between 0 and %d kgs.\n", MAX_LUGGAGE_WEIGHT);
                }
            }

            while (1) {
                printf("Enter Body Weight for Passenger %d: ", i + 1);
                scanf("%d", &body_weight);
                if (body_weight >= MIN_BODY_WEIGHT && body_weight <= MAX_BODY_WEIGHT) {
                    break;
                } else {
                    printf("Error: Body weight should be between %d and %d kgs.\n", MIN_BODY_WEIGHT, MAX_BODY_WEIGHT);
                }
            }
            passenger_weights[i] = body_weight + luggage_weight;
        }
        for (int i = 0; i < num_passengers; i++) {
            if (pipe(pfd[i]) == -1) {
                fprintf(stderr, "Pipe Failed");
                return 1;
            }
            passenger_pid[i] = fork();
            if (passenger_pid[i] < 0) {
                fprintf(stderr, "Fork Failed");
                return 1;
            } else if (passenger_pid[i] == 0) {
                // passenger process logic
                int weight = passenger_weights[i];
                close(pfd[i][READ_END]); // Close read end in child
                write(pfd[i][WRITE_END], &weight, sizeof(weight));
                close(pfd[i][WRITE_END]); // Close write end after use
                exit(0); // Ensure child process terminates after writing to pipe
            }
        }


            int crew_weight = NUM_CREW_MEMBERS * AVG_CREW_WEIGHT; // This should be 525 kgs based on your manual calculation
            for (int i = 0; i < num_passengers; i++)
                {
                    wait(NULL);
                } // Wait for the child to finish before
            // Corrected total passenger weight calculation
            for (int i = 0; i < num_passengers; i++) {
                int temp[1];
                close(pfd[i][WRITE_END]); // Close write end in parent
		        read(pfd[i][READ_END], temp, sizeof(temp));
                total_passenger_weight += temp[0];
                close(pfd[i][READ_END]); // Close read end after use
            }


            // Correctly add the crew weight to the total passenger weight
            total_passenger_weight += crew_weight; // Add crew weight
            printf("Total weight of the plane: %d kgs\n", total_passenger_weight);


    }

    else if(plane_type == 0){
        int num_cargo_items;
        int avg_cargo_weight;
        int total_cargo_weight = 0;
        int crew_member_weight = 75;
        int num_crew_members = 2;

        printf("Enter Number of Cargo Items: ");
        scanf("%d", &num_cargo_items);

        if (num_cargo_items < 1 || num_cargo_items > 100) {
            printf("Error: Number of cargo items should be between 1 and 100.\n");
            return 1;
        }

        printf("Enter Average Weight of Cargo Items: ");
        scanf("%d", &avg_cargo_weight);

        if (avg_cargo_weight < 1 || avg_cargo_weight > 100) {
            printf("Error: Average weight of cargo items should be between 1 and 100.\n");
            return 1;
        }
        total_cargo_weight = avg_cargo_weight * num_cargo_items;
        total_passenger_weight = total_cargo_weight + (crew_member_weight * num_crew_members);
        printf("Total weight of the plane: %d kgs\n", total_passenger_weight);

    }   
    int airport_departure,airport_arrival;

     while (1) {
                printf("Enter Airport Number for Departure: ");
                scanf("%d", &airport_departure);
                if (airport_departure >= MIN_AIRPORT && airport_departure <= MAX_AIRPORT) {
                    break;
                } else {
                    printf("Error: Airport Number should be between %d and %d kgs.\n", MIN_AIRPORT, MAX_AIRPORT);
                }
            }
     while (1) {
                printf("Enter Airport Number for Arrival: ");
                scanf("%d", &airport_arrival);
                if (airport_arrival >= MIN_AIRPORT && airport_arrival <= MAX_AIRPORT) {
                    break;
                } else {
                    printf("Error: Airport Number should be between %d and %d kgs.\n", MIN_AIRPORT, MAX_AIRPORT);
                }
            }
 
    int msgqid;
    key_t msgkey;
    msgkey = ftok("plane.c", 'A');
    msgqid = msgget(msgkey, IPC_CREAT | 0666);
    if (msgqid == -1) {
    perror("Failed to create message queue");
    exit(1);
    }

    struct msg_t message;
    message.mtype = 21;
    message.msg_text[0]=plane_id;
    message.msg_text[1]=airport_arrival;
    message.msg_text[2]=airport_departure;
    message.msg_text[3]=plane_id;
    message.msg_text[4]=total_passenger_weight;
    message.msg_text[5]=num_passengers;
    message.msg_text[6]=plane_type;
    if (msgsnd(msgqid, &message,MAXLEN,0) == -1){
        printf("error in sending message\n");
        exit(1);
    }
    struct msg_t received_msg;
    while(1){
        int rc = msgrcv(msgqid, (void *)&message, MAXLEN, plane_id, IPC_NOWAIT);
            if (rc == -1) {
                if (errno == ENOMSG) {
                    printf("\rTravelling");
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
                printf("\nPlane %d has successfully traveled from Airport %d to Airport %d\n", plane_id, message.msg_text[2], message.msg_text[1]-10);
                break;
            }
    }
    return 0;
        
}
