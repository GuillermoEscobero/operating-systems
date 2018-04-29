#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "queue.h"

typedef struct queue {
        int front, rear;

        int size;
        struct plane **elements;
} Queue;

Queue *q;

pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
// pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t queue_full_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t queue_empty_cond = PTHREAD_COND_INITIALIZER;

// int queue_flag_operation = 0;
int queue_flag_full = 0;
int queue_flag_empty = 1;

/*To create a queue*/
int queue_init(int size){
        q = (Queue*)malloc(sizeof(Queue));

        q->front = -1;
        q->rear = -1;

        q->size = size;
        q->elements = (struct plane**)malloc(sizeof(struct plane*));

        for (int i = 0; i < size; i++) {
                q->elements[i] = (struct plane*)malloc(sizeof(struct plane));
        }
        printf("[QUEUE] Buffer initialized\n");

        return 0;
}

/* To Enqueue an element*/
int queue_put(struct plane* x) {
        pthread_mutex_lock(&queue_lock);
        printf("[QUEUE] Storing plane with id %d\n", x->id_number);

        // while (queue_flag_operation == 1) {
        //         pthread_cond_wait(&queue_cond, &queue_lock);
        // }
        // queue_flag_operation = 1;

        while (queue_flag_full == 1) {
          pthread_cond_wait(&queue_full_cond, &queue_lock);
        }
        /*
        if (queue_full() == 1) {
                printf("Full queue\n");
                // queue_flag_operation = 0;
                // pthread_cond_signal(&queue_cond);
                pthread_mutex_unlock(&queue_lock);
                return -1;
        } else */
        if (q->front == -1) {
                q->front = q->rear = 0;
                q->elements[q->rear] = x;
        } else if (q->rear == q->size-1 && q->front != 0) {
                q->rear = 0;
                q->elements[q->rear] = x;
        } else {
                q->rear++;
                q->elements[q->rear] = x;
        }

        // queue_flag_operation = 0;
        queue_flag_empty = 0;
        if (queue_full() == 1) {
          queue_flag_full = 1;
        }

        //pthread_cond_signal(&queue_cond);
        pthread_cond_signal(&queue_empty_cond);
        pthread_mutex_unlock(&queue_lock);
        return 0;
}


/* To Dequeue an element.*/
struct plane* queue_get(void) {
        pthread_mutex_lock(&queue_lock);

        // while (queue_flag_operation == 1) {
        //         pthread_cond_wait(&queue_cond, &queue_lock);
        // }
        while (queue_flag_empty == 1) {
          pthread_cond_wait(&queue_empty_cond, &queue_lock);
        }


        // queue_flag_operation = 1;

        // if (queue_empty() == 1) {
        //         printf("Empty queue\n");
        //         queue_flag_operation = 0;
        //         pthread_cond_signal(&queue_cond);
        //         pthread_mutex_unlock(&queue_lock);
        //         return NULL;
        // }

        struct plane *element = q->elements[q->front];
        q->elements[q->front] = NULL;

        if (q->front == q->rear) {
                q->front = -1;
                q->rear = -1;
        } else if (q->front == q->size-1) {
                q->front = 0;
        } else {
                q->front++;
        }
        printf("[QUEUE] Getting plane with id %d\n", element->id_number);

        // queue_flag_operation = 0;

        if (queue_empty() == 1) {
          queue_flag_empty = 1;
        }

        // pthread_cond_signal(&queue_cond);
        pthread_mutex_unlock(&queue_lock);

        return element;
}


/*To check queue state*/
int queue_empty(void){
        // pthread_mutex_lock(&queue_lock);
        // while (queue_flag_operation == 1) {
        //         pthread_cond_wait(&queue_cond, &queue_lock);
        // }
        // queue_flag_operation = 1;

        if (q->front == -1) {
                /* Queue empty */
                // printf("Empty queue\n");
                // queue_flag_operation = 0;
                // pthread_cond_signal(&queue_cond);
                // pthread_mutex_unlock(&queue_lock);
                return 1;
        }
        /* Queue not empty */
        // queue_flag_operation = 0;
        // pthread_cond_signal(&queue_cond);
        // pthread_mutex_unlock(&queue_lock);
        return 0;
}

/*To check queue state*/
int queue_full(void){
        // pthread_mutex_lock(&queue_lock);
        // while (queue_flag_operation == 1) {
        //         pthread_cond_wait(&queue_cond, &queue_lock);
        // }
        // queue_flag_operation = 1;

        if ((q->front == 0 && q->rear == q->size-1) ||
            (q->rear == (q->front-1)%(q->size-1))) {
                printf("Full queue\n");
                // queue_flag_operation = 0;
                // pthread_cond_signal(&queue_cond);
                // pthread_mutex_unlock(&queue_lock);
                return 1;
        }
        /* Queue not full */
        // queue_flag_operation = 0;
        // pthread_cond_signal(&queue_cond);
        // pthread_mutex_unlock(&queue_lock);
        return 0;
}

/*To destroy the queue and free the resources*/
int queue_destroy(void){
        int i;
        for (i = 0; i < q->size; i++) {
                free(q->elements[i]);
                q->elements[i] = NULL;
        }
        free(q->elements);
        q->elements = NULL;
        free(q);
        q = NULL;

        return 0;
}

void print_plane(struct plane* pln) {
        printf("id_number = %d\n", pln->id_number);
        printf("time_action = %d\n", pln->time_action);
        printf("action = %d\n", pln->action);
        printf("last_flight = %d\n", pln->last_flight);
}

void display_queue(void) {
        int i;
        if (queue_empty() == 1) {
                return;
        }

        printf("Elements in circular queue are:\n");
        if (q->rear >= q->front) {
                for (i = q->front; i <= q->rear; i++) {
                        printf("PLANE IN POSITION %d\n", i);
                        print_plane(q->elements[i]);
                }
        } else {
                for (i = q->front; i < q->size; i++) {
                        printf("PLANE IN POSITION %d\n", i);
                        print_plane(q->elements[i]);
                }
                for (i = 0; i <= q->rear; i++) {
                        printf("PLANE IN POSITION %d\n", i);
                        print_plane(q->elements[i]);
                }
        }

}
