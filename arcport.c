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

pthread_mutex_t main_mutex;

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
    printf("[CONTROL] Track is free for plane with id %d\n", pln->id_number);
    if (pln->last_flight == 1) {
        printf("[CONTROL] After plane with id %d the airport will be closed\n", pln->id_number);
    }
    //FIXME: aqui va a haber peleas con el id de vuelo
    sleep(pln->time_action);
    printf("[CONTROL] Plane %d landed in %d seconds$$$$$$$$$$$$$$$$$$$\n", pln->id_number, pln->time_action);
    served_landings++;

    return 0;
}

int serve_takeoff(struct plane *pln) {
    printf("[CONTROL] Putting plane with id %d in track\n", pln->id_number);
    if (pln->last_flight == 1) {
        printf("[CONTROL] After plane with id %d the airport will be closed\n", pln->id_number);
    }
    //FIXME: aqui va a haber peleas con el id de vuelo, despues de hacer el sleep se cambia el pln
    sleep(pln->time_action);
    printf("[CONTROL] Plane %d took off after %d seconds$$$$$$$$$$$$$$$$$\n", pln->id_number, pln->time_action);
    served_takeoffs++;

    return 0;
}

void track_manager(void) {
    int i;
    //struct plane *pln = (struct plane *) malloc(sizeof(struct plane));

    for (i = 0; i < planes_takeoff; i++) {
        pthread_mutex_lock(&main_mutex);
        struct plane *pln = (struct plane *) malloc(sizeof(struct plane));
        pln->id_number = next_id;
        pln->time_action = time_takeoff;
        pln->action = OP_TAKEOFF;

        if (next_id == (planes_land + planes_takeoff - 1)) {
            pln->last_flight = 1;
        } else {
            pln->last_flight = 0;
        }
        next_id++;
        printf("[TRACKBOSS] Plane with id %d checked\n", pln->id_number);

        queue_put(pln);
        //free(pln);
        printf("[TRACKBOSS] Plane with id %d ready to takeoff\n", pln->id_number);
        pthread_mutex_unlock(&main_mutex);

    }
    pthread_exit(0);
}

void radar(void) {
    int i;
    //struct plane *pln = (struct plane *) malloc(sizeof(struct plane));

    for (i = 0; i < planes_land; i++) {
        pthread_mutex_lock(&main_mutex);
        struct plane *pln = (struct plane *) malloc(sizeof(struct plane));
        pln->id_number = next_id;
        pln->time_action = time_landing;
        pln->action = OP_LAND;

        if (pln->id_number == (planes_land + planes_takeoff - 1)) {
            pln->last_flight = 1;
        } else {
            pln->last_flight = 0;
        }
        next_id++;
        printf("[RADAR] Plane with id %d detected!\n", pln->id_number);
        queue_put(pln);
        printf("[RADAR] Plane with id %d ready to land\n", pln->id_number);
        //free(pln);
        pthread_mutex_unlock(&main_mutex);
    }
    pthread_exit(0);
}


void tower(void) {
    int last_flight = 0;
    while (last_flight == 0) {
        struct plane *pln;

        //FIXME: no pilla bien el plane que le toca en cada momento, se los salta o yo que se
        pln = queue_get();
        //print_plane(pln);
        //display_queue();

        if (pln == NULL) {
            fprintf(stderr, "ERROR something went really wrong\n");
            pthread_exit(0);
        }

        if (pln->action == OP_TAKEOFF) {
            serve_takeoff(pln);
        } else {
            serve_landing(pln);
        }

        last_flight = pln->last_flight;
        free(pln);

    }

    printf("Airport closed!\n");
    printf("SERVED FLIGHTS: %d\n", served_landings + served_takeoffs);
    pthread_exit(0);
}

int main(int argc, char **argv) {

    if (argc != 1 && argc != 6) {
        fprintf(stderr, "%s\n\n", "usage 1: ./arcport");
        fprintf(stderr, "%s\n\n",
                "usage 2: ./arcport <n_planes_takeoff> <time_to_takeoff> <n_planes_to_arrive> <time_to_arrive> <size_of_buffer>");
        return -1;
    }

    print_banner();

    if (argc != 1) {
        planes_takeoff = atoi(argv[1]);
        time_takeoff = atoi(argv[2]);
        planes_land = atoi(argv[3]);
        time_landing = atoi(argv[4]);
        size = atoi(argv[5]);
    }

    pthread_mutex_init(&main_mutex, NULL);

    //waiting = planes_takeoff + planes_land;

    queue_init(size);

    pthread_t pid[3];
    pthread_create(&pid[0], NULL, (void *) track_manager, (void *) &planes_takeoff);
    pthread_create(&pid[1], NULL, (void *) radar, (void *) &planes_land);
    pthread_create(&pid[2], NULL, (void *) tower, NULL);

    pthread_join(pid[0], NULL);
    pthread_join(pid[1], NULL);
    pthread_join(pid[2], NULL);

    print_end();

    queue_destroy();
    pthread_mutex_destroy(&main_mutex);

    return 0;
}
