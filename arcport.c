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

pthread_mutex_t mut_id;
pthread_mutex_t data_mutex;
pthread_cond_t queue_not_full, queue_not_empty;

int planes_takeoff = DEFAULT_PLANES_TAKEOFF;
int time_takeoff = DEFAULT_TIME_TAKEOFF;
int planes_land = DEFAULT_PLANES_LAND;
int time_landing = DEFAULT_TIME_LANDING;
int size = DEFAULT_SIZE;

int next_id = 0;
int served_landings = 0;
int served_takeoffs = 0;

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

void track_manager(void) {
  int i;
  struct plane *pln = (struct plane*)malloc(sizeof(struct plane));

  for (i = 0; i < planes_takeoff; i++) {
    pln->id_number = next_id;
    pln->time_action = time_takeoff;
    pln->action = OP_TAKEOFF;

    if (next_id == (planes_land+planes_takeoff-1)) {
      pln->last_flight = 1;
    } else {
      pln->last_flight = 0;
    }

    pthread_mutex_lock(&data_mutex);
    next_id++;


    printf("[TRACKBOSS] Plane with id %d checked\n", pln->id_number);
    while (queue_full() == 1) {
      pthread_cond_wait(&queue_not_full, &data_mutex);
    }
    queue_put(pln);
    printf("[TRACKBOSS] Plane with id %d ready to takeoff\n", pln->id_number);
    pthread_cond_signal(&queue_not_empty);

    pthread_mutex_unlock(&data_mutex);

  }
  free(pln);
  pthread_exit(0);
}

void radar(void) {
  int i;
  struct plane *pln = (struct plane*)malloc(sizeof(struct plane));

  for (i = 0; i < planes_land; i++) {

    pln->id_number = next_id;
    pln->time_action = time_landing;
    pln->action = OP_LAND;

    if (pln->id_number == (planes_land+planes_takeoff-1)) {
      pln->last_flight = 1;
    } else {
      pln->last_flight = 0;
    }

    pthread_mutex_lock(&data_mutex);
    next_id++;


    printf("[RADAR] Plane with id %d detected!\n", pln->id_number);
    while (queue_full() == 1) {
      pthread_cond_wait(&queue_not_full, &data_mutex);
    }
    queue_put(pln);

    printf("[RADAR] Plane with id %d ready to land\n", pln->id_number);

    pthread_cond_signal(&queue_not_empty);
    pthread_mutex_unlock(&data_mutex);
  }

  free(pln);
  pthread_exit(0);
}

int serve_landing(struct plane *pln) {
  struct plane pln_local;
  memcpy(&pln_local, pln, sizeof(struct plane));

  printf("[CONTROL] Track is free for plane with id %d\n", pln_local.id_number);

  if (pln_local.last_flight == 1) {
    printf("[CONTROL] After plane with id %d the airport will be closed\n", pln_local.id_number);
  }
  sleep(pln_local.time_action);
  printf("[CONTROL] Plane %d landed in %d seconds$$$$$$$$$$$$$$$$$$$\n", pln_local.id_number, pln_local.time_action);
  served_landings++;

  return 0;
}

int serve_takeoff(struct plane *pln) {
  struct plane pln_local;
  memcpy(&pln_local, pln, sizeof(struct plane));

  printf("[CONTROL] Putting plane with id %d in track\n", pln_local.id_number);

  if (pln_local.last_flight == 1) {
    printf("[CONTROL] After plane with id %d the airport will be closed\n", pln_local.id_number);
  }
  sleep(pln_local.time_action);
  printf("[CONTROL] Plane %d took off after %d seconds$$$$$$$$$$$$$$$$$\n", pln_local.id_number, pln_local.time_action);
  served_takeoffs++;

  return 0;
}

void tower(void) {
  // struct plane *last = (struct plane*)malloc(sizeof(struct plane));

  while (served_landings+served_takeoffs < planes_land+planes_takeoff) {
    pthread_mutex_lock(&data_mutex);

    while (queue_empty() == 1) {
      printf("[CONTROL] Waiting for planes in empty queue\n");
      pthread_cond_wait(&queue_not_empty, &data_mutex);
    }

    pln = queue_get();

    pthread_cond_signal(&queue_not_full);
    pthread_mutex_unlock(&data_mutex);

    if (pln == NULL) {
      fprintf(stderr, "ERROR something go really wrong\n");
      pthread_exit(0);
    }

    if (pln->action == OP_TAKEOFF) {
      serve_takeoff(pln);
    } else {
      serve_landing(pln);
    }

    // if (pln->last_flight == 1) {
    //   break;
    // }

  }

  //free(pln);
  printf("Airport closed!\n");
  printf("SERVED FLIGHTS: %d\n", served_landings+served_takeoffs);
  pthread_exit(0);
}

int main(int argc, char ** argv) {

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

    pthread_mutex_init(&mut_id, NULL);
    pthread_mutex_init(&data_mutex, NULL);
    pthread_cond_init(&queue_not_full, NULL);
    pthread_cond_init(&queue_not_empty, NULL);

    //waiting = planes_takeoff + planes_land;

    queue_init(size);

    pthread_t pid[3];
    pthread_create(&pid[0], NULL, (void*)track_manager, (void*)&planes_takeoff);
    pthread_create(&pid[1], NULL, (void*)radar, (void*)&planes_land);
    pthread_create(&pid[2], NULL, (void*)tower, NULL);

    pthread_join(pid[0], NULL);
    pthread_join(pid[1], NULL);
    pthread_join(pid[2], NULL);

	  print_end();

    queue_destroy();
    pthread_mutex_destroy(&mut_id);
    pthread_cond_destroy(&queue_not_full);
    pthread_cond_destroy(&queue_not_empty);

    return 0;
}
