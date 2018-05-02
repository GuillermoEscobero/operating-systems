#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include <pthread.h>
#include <unistd.h>

#include <string.h>

#define NUM_TRACKS 1

#define OP_TAKEOFF 0
#define OP_LAND 1

#define DEFAULT_PLANES_TAKEOFF 4
#define DEFAULT_PLANES_LAND 3
#define DEFAULT_TIME_TAKEOFF 2
#define DEFAULT_TIME_LANDING 3
#define DEFAULT_SIZE 6

pthread_mutex_t main_mutex = PTHREAD_MUTEX_INITIALIZER;

int planes_takeoff = DEFAULT_PLANES_TAKEOFF;
int time_takeoff = DEFAULT_TIME_TAKEOFF;
int planes_land = DEFAULT_PLANES_LAND;
int time_landing = DEFAULT_TIME_LANDING;
int size = DEFAULT_SIZE;

int next_id = 0;
int served_landings = 0;
int served_takeoffs = 0;

void print_banner() {
    printf("\n*****************************************\n");
    printf("Welcome to ARCPORT - The ARCOS AIRPORT.\n");
    printf("*****************************************\n\n");
}

void print_end() {
    printf("\n********************************************\n ---> Thanks for your trust in us <---\n"\
             "********************************************\n");
}

int serve_landing(struct plane *pln) {
    /* Land the plane taken as parameter and print the messages required */
    printf("[CONTROL] Track is free for plane with id %d\n", pln->id_number);

    /* If the plane is the last one, print the message showing it */
    if (pln->last_flight == 1) {
        printf("[CONTROL] After plane with id %d the airport will be closed\n", pln->id_number);
    }
    /* Simulate the landing time using sleep() with the duration in the plane structure */
    sleep((unsigned int) pln->time_action);
    printf("[CONTROL] Plane %d landed in %d seconds\n", pln->id_number, pln->time_action);
    
    /* Increase the counter of planes landed to track them in order to print later a file with the info */
    served_landings++;

    return 0;
}

int serve_takeoff(struct plane *pln) {
    /* Takeoff the plane taken as parameter and print the messages required */
    printf("[CONTROL] Putting plane with id %d in track\n", pln->id_number);

    /* If the plane is the last one, print the message showing it */
    if (pln->last_flight == 1) {
        printf("[CONTROL] After plane with id %d the airport will be closed\n", pln->id_number);
    }
    /* Simulate the takeoff time using sleep() with the duration in the plane structure */
    sleep((unsigned int) pln->time_action);
    printf("[CONTROL] Plane %d took off after %d seconds\n", pln->id_number, pln->time_action);

    /* Increase the counter of planes that took off to track them in order to print later a file with the info */
    served_takeoffs++;

    return 0;
}

void track_manager(void) {
    int i;
    /* Loop through the number of planes to be created received as arguments */
    for (i = 0; i < planes_takeoff; i++) {
        /* Protect the creation of the plane in order to be able to assure that
         * no other one has the same id as the one being created by this thread */
        pthread_mutex_lock(&main_mutex);

        struct plane *pln = (struct plane *) malloc(sizeof(struct plane));
        pln->id_number = next_id;
        pln->time_action = time_takeoff;
        pln->action = OP_TAKEOFF;

        /* If the plane is the last one, set the last_flight flag to one */
        if (next_id == (planes_land + planes_takeoff - 1)) {
            pln->last_flight = 1;
        } else {
            pln->last_flight = 0;
        }
        /* Increment the plane counter and print the message required */
        next_id++;
        printf("[TRACKBOSS] Plane with id %d checked\n", pln->id_number);

        /* Store the plane into the queue and print the required message */
        queue_put(pln);
        printf("[TRACKBOSS] Plane with id %d ready to takeoff\n", pln->id_number);

        /* Unlock the mutex to allow other threads to create more planes */
        pthread_mutex_unlock(&main_mutex);

    }
    pthread_exit(0);
}

void radar(void) {
    int i;
    /* Loop through the number of planes to be created received as arguments */
    for (i = 0; i < planes_land; i++) {
        /* Protect the creation of the plane in order to be able to assure that
         * no other one has the same id as the one being created by this thread */
        pthread_mutex_lock(&main_mutex);

        struct plane *pln = (struct plane *) malloc(sizeof(struct plane));
        pln->id_number = next_id;
        pln->time_action = time_landing;
        pln->action = OP_LAND;

        /* If the plane is the last one, set the last_flight flag to one */
        if (pln->id_number == (planes_land + planes_takeoff - 1)) {
            pln->last_flight = 1;
        } else {
            pln->last_flight = 0;
        }
        /* Increment the plane counter and print the message required */
        next_id++;
        printf("[RADAR] Plane with id %d detected!\n", pln->id_number);

        /* Store the plane into the queue and print the required message */
        queue_put(pln);
        printf("[RADAR] Plane with id %d ready to land\n", pln->id_number);

        /* Unlock the mutex to allow other threads to create more planes */
        pthread_mutex_unlock(&main_mutex);
    }
    pthread_exit(0);
}

void tower(void) {
    /* Initialize the flag to 0 */
    int last_flight = 0;

    /* Loop until the last plane is served */
    while (last_flight == 0) {
        struct plane *pln;

        /* Get the first plane from the queue */
        pln = queue_get();

        if (pln == NULL) {
            fprintf(stderr, "ERROR something went really wrong\n");
            pthread_exit(0);
        }

        /* Depending on where is the plane (air or land) takeoff or land it */
        if (pln->action == OP_TAKEOFF) {
            serve_takeoff(pln);
        } else {
            serve_landing(pln);
        }

        /* Update the flag to the one in the plane served in this iteration, if it is the last one, the loop will end */
        last_flight = pln->last_flight;
        free(pln);

    }

    printf("Airport closed!\n");
    pthread_exit(0);
}

int create_output_file(void) {
    /* Create a file if it does not exists or open it if it does */
    FILE *file_pointer = fopen("resume.air", "w+");

    /* Print an error if there was any problem */
    if (file_pointer == NULL) {
        fprintf(stderr, "%s\n", "ERROR something happened while opening/creating the file");
        return -1;
    }

    /* Print the requested message into the file */
    fprintf(file_pointer, "\t%s %d\n", "Total number of planes processed:", served_landings + served_takeoffs);
    fprintf(file_pointer, "\t%s %d\n", "Number of planes landed:", served_landings);
    fprintf(file_pointer, "\t%s %d\n", "Number of planes taken off:", served_takeoffs);

    /* Close the file */
    if (fclose(file_pointer) != 0){
        fprintf(stderr, "%s\n", "ERROR while closing the file");
        return -1;
    }

    return 0;
}

int main(int argc, char **argv) {

    if (argc != 1 && argc != 6) {
        fprintf(stderr, "%s\n\n", "usage 1: ./arcport");
        fprintf(stderr, "%s\n\n",
                "usage 2: ./arcport <n_planes_takeoff> <time_to_takeoff> <n_planes_to_arrive> <time_to_arrive> <size_of_buffer>");
        return -1;
    }

    print_banner();

    /* Parse arguments */
    if (argc != 1) {
        planes_takeoff = (int) strtol(argv[1], NULL, 10);
        time_takeoff = (int) strtol(argv[2], NULL, 10);
        planes_land = (int) strtol(argv[3], NULL, 10);
        time_landing = (int) strtol(argv[4], NULL, 10);
        size = (int) strtol(argv[5], NULL, 10);
    }

    /* Initialize the circular queue */
    queue_init(size);

    /* Create an array of threads to save the thread id and create three of them with a function to execute */
    pthread_t tid[3];

    if (pthread_create(&tid[0], NULL, (void *) track_manager, (void *) &planes_takeoff) != 0) {
        fprintf(stderr, "%s\n", "ERROR the first thread couldn't be created, check errno macro for more details");
        return -1;
    }
    if (pthread_create(&tid[1], NULL, (void *) radar, (void *) &planes_land) != 0) {
        fprintf(stderr, "%s\n", "ERROR the second thread couldn't be created, check errno macro for more details");
        return -1;
    }
    if (pthread_create(&tid[2], NULL, (void *) tower, NULL) != 0) {
        fprintf(stderr, "%s\n", "ERROR the last thread couldn't be created, check errno macro for more details");
        return -1;
    }

    /* Join the threads after they exit */
    if (pthread_join(tid[0], NULL) != 0) {
        fprintf(stderr, "%s\n", "ERROR while waiting for the first thread");
        return -1;
    }
    if (pthread_join(tid[1], NULL) != 0) {
        fprintf(stderr, "%s\n", "ERROR while waiting for the second thread");
        return -1;
    }
    if (pthread_join(tid[2], NULL) != 0) {
        fprintf(stderr, "%s\n", "ERROR while waiting for the last thread");
        return -1;
    }

    print_end();

    /* Destroy the queue and the mutex */
    queue_destroy();

    if (pthread_mutex_destroy(&main_mutex) != 0) {
        fprintf(stderr, "%s\n", "ERROR when destroying the main mutex");
        return -1;
    }

    create_output_file();

    return 0;
}
