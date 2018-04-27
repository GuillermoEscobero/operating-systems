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

int track_manager(int n) {
  int i;
  struct plane *pln = (struct plane*)malloc(sizeof(struct plane));

  for (i = 0; i < n; i++) {
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
    sleep(pln->time_action);
    printf("[TRACKBOSS] Plane with id %d ready to takeoff\n", pln->id_number);

    waiting--;
    next_id++;

    pthread_mutex_unlock(&mut_id);
  }
  return 0;
}

int radar(int n) {
  int i;
  struct plane *pln = (struct plane*)malloc(sizeof(struct plane));

  for (i = 0; i < n; i++) {
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
    sleep(pln->time_action);
    printf("[RADAR] Plane with id %d ready to land\n", pln->id_number);

    waiting--;
    next_id++;

    pthread_mutex_unlock(&mut_id);
  }
  return 0;
}

int main(int argc, char ** argv) {
    waiting = planes_takeoff + planes_land;

    print_banner();

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

    queue_init(DEFAULT_SIZE);

    pthread_t pid[3];

    pthread_create(&pid[0], NULL, (void*)track_manager, (void*)DEFAULT_PLANES_TAKEOFF);
    pthread_create(&pid[1], NULL, (void*)radar, (void*)DEFAULT_PLANES_LAND);

    pthread_join(pid[0], NULL);
    pthread_join(pid[1], NULL);

	print_end();

    return 0;
}
