#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include <pthread.h>
#include <unistd.h>

#define NUM_TRACKS 1

#define OP_TAKEOFF 0
#define OP_LAND 1

#define DEFAULT_PLANES_TAKEOFF 4
#define DEFAULT_PLANES_LAND 3
#define DEFAULT_TIME_TAKEOFF 2
#define DEFAULT_TIME_LANDING 3
#define DEFAULT_SIZE 6

pthread_mutex_t mut_id = PTHREAD_MUTEX_INITIALIZER;

int planes_takeoff = DEFAULT_PLANES_TAKEOFF;
int time_takeoff = DEFAULT_TIME_TAKEOFF;
int planes_land = DEFAULT_PLANES_LAND;
int time_landing = DEFAULT_TIME_LANDING;
int size = DEFAULT_SIZE;

int next_id = 0;
int waiting = 0;

void print_banner()
{
    printf("\n*****************************************\n");
    printf("Welcome to ARCPORT - The ARCOS AIRPORT.\n");
    printf("*****************************************\n\n");
}

void print_end()
{
	printf("\n********************************************\n ---> Thanks for your trust in us <---\n"\
             "********************************************\n");
}

int track_manager(void) {
  int i;
  struct plane *pln = (struct plane*)malloc(sizeof(struct plane));

  for (i = 0; i < planes_takeoff; i++) {
    pthread_mutex_lock(&mut_id);

    pln->id_number = next_id;
    pln->time_action = time_takeoff;
    pln->action = OP_TAKEOFF;

    if (waiting == 1) {
      pln->last_flight = 1;
    } else {
      pln->last_flight = 0;
    }

    printf("[TRACKBOSS] Plane with id %d checked\n", pln->id_number);
    queue_put(pln);
    // sleep(pln->time_action);
    printf("[TRACKBOSS] Plane with id %d ready to takeoff\n", pln->id_number);

    waiting--;
    next_id++;

    pthread_mutex_unlock(&mut_id);
  }
  free(pln);
  return 0;
}

int radar(void) {
  int i;
  struct plane *pln = (struct plane*)malloc(sizeof(struct plane));

  for (i = 0; i < planes_land; i++) {
    pthread_mutex_lock(&mut_id);

    pln->id_number = next_id;
    pln->time_action = time_landing;
    pln->action = OP_LAND;

    if (waiting == 1) {
      pln->last_flight = 1;
    } else {
      pln->last_flight = 0;
    }

    printf("[RADAR] Plane with id %d detected!\n", pln->id_number);
    queue_put(pln);
    // sleep(pln->time_action);
    printf("[RADAR] Plane with id %d ready to land\n", pln->id_number);

    waiting--;
    next_id++;

    pthread_mutex_unlock(&mut_id);
  }

  free(pln);
  return 0;
}

int tower(void) {
  struct plane *pln = (struct plane*)malloc(sizeof(struct plane));
  int last_flight = 0;
  int served = 0;

  while (last_flight == 0) {
    pthread_mutex_lock(&mut_id);

    if (queue_empty()) {
      printf("[CONTROL] Waiting for planes in empty queue\n");
    }
    pln = queue_get();

    if (pln == NULL) {
      fprintf(stderr, "ERROR something go really wrong\n");
      return -1;
    }

    switch (pln->action) {
      case OP_TAKEOFF:
          printf("[CONTROL] Putting plane with id %d in track\n", pln->id_number);

          if (pln->last_flight == 1) {
            printf("[CONTROL] After plane with id %d the airport will be closed\n", pln->id_number);
          }
          sleep(pln->time_action);
          printf("[CONTROL] Plane %d took off after %d seconds\n", pln->id_number, pln->time_action);
          break;
      case OP_LAND:
          printf("[CONTROL] Track is free for plane with id %d\n", pln->id_number);

          if (pln->last_flight == 1) {
            printf("[CONTROL] After plane with id %d the airport will be closed\n", pln->id_number);
          }
          sleep(pln->time_action);
          printf("[CONTROL] Plane %d landed in %d seconds\n", pln->id_number, pln->time_action);
          break;
      default:
          fprintf(stderr, "ERROR undefined operation code\n");
    }
    last_flight = pln->last_flight;
    served++;
    pthread_mutex_unlock(&mut_id);


  }
  printf("Airport closed!\n");
  return 0;
}

int main(int argc, char ** argv) {

    /* Testing lines while implementing */
    // queue_init (5);
    // printf("queue_empty() = %d\n", queue_empty());
    // printf("queue_full() = %d\n", queue_full());
    //
    // struct plane iberia = {
    //   .id_number = 123,
    //   .time_action = 5548,
    //   .action = 4,
    //   .last_flight = 556,
    // };
    //
    // struct plane *caramel;
    // queue_put (&iberia);
    // printf("queue_empty() = %d\n", queue_empty());
    // printf("queue_full() = %d\n", queue_full());
    // display_queue();
    // caramel = queue_get();
    // printf("queue_empty() = %d\n", queue_empty());
    // printf("queue_full() = %d\n", queue_full());
    //
    // printf("\nPLANE caramel\n");
    // printf("id_number = %d\n", caramel->id_number);
    // printf("time_action = %d\n", caramel->time_action);
    // printf("action = %d\n", caramel->action);
    // printf("last_flight = %d\n", caramel->last_flight);
    // queue_destroy();

    /* FINISH TESTING LINES */

    if (argc != 1 && argc != 6) {
      fprintf(stderr, "%s\n\n", "usage 1: ./arcport");
      fprintf(stderr, "%s\n\n", "usage 2: ./arcport <n_planes_takeoff> <time_to_takeoff> <n_planes_to_arrive> <time_to_arrive> <size_of_buffer>");
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

    waiting = planes_takeoff + planes_land;

    queue_init(size);

    pthread_t pid[3];
    pthread_create(&pid[0], NULL, (void*)track_manager, (void*)&planes_takeoff);
    pthread_create(&pid[1], NULL, (void*)radar, (void*)&planes_land);
    pthread_create(&pid[2], NULL, (void*)tower, NULL);

    pthread_join(pid[0], NULL);
    pthread_join(pid[1], NULL);
    pthread_join(pid[2], NULL);

	  print_end();

    return 0;
}
