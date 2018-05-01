#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "queue.h"

pthread_mutex_t queue_mutex;
pthread_cond_t queue_not_empty, queue_not_full;

typedef struct queue {
    int front, rear;

    int size;
    struct plane **elements;
} Queue;

Queue *q;

/*To create a queue*/
int queue_init(int size) {
    q = (Queue *) malloc(sizeof(Queue));
    if (!q) {
        fprintf(stderr, "ERROR while initializing queue\n");
        return -1;
    }
    q->front = -1;
    q->rear = -1;

    q->size = size;
    q->elements = (struct plane **) malloc(size * sizeof(struct plane *));

    if (!q->elements) {
        fprintf(stderr, "ERROR while initializing queue\n");
        return -1;
    }
    printf("[QUEUE] Buffer initialized\n");

    return 0;
}

/* To Enqueue an element*/
int queue_put(struct plane *x) {
    pthread_mutex_lock(&queue_mutex);
    while (queue_full() == 1) {
        pthread_cond_wait(&queue_not_full, &queue_mutex);
    }
    printf("[QUEUE] Storing plane with id %d\n", x->id_number);

    q->rear = (q->rear + 1) % q->size;
    q->elements[q->rear] = x;
    if (q->front == -1) {
        q->front = q->rear;
    }
    pthread_cond_signal(&queue_not_empty);
    pthread_mutex_unlock(&queue_mutex);
    return 0;
}


/* To Dequeue an element.*/
struct plane *queue_get(void) {
    //FIXME: esto no deberia ir aqui
    while (queue_empty() == 1) {
        pthread_cond_wait(&queue_not_empty, &queue_mutex);
        printf("[CONTROL] Waiting for planes in empty queue\n");
    }
    pthread_mutex_lock(&queue_mutex);
    struct plane *element = q->elements[q->front];
    if (q->front == q->rear) {
        q->front = -1;
        q->rear = -1;
    } else {
        q->front = (q->front + 1) % q->size;
    }
    printf("[QUEUE] Getting plane with id %d\n", element->id_number);
    pthread_cond_signal(&queue_not_full);
    pthread_mutex_unlock(&queue_mutex);
    return element;
}


/*To check queue state*/
int queue_empty(void) {
    pthread_mutex_lock(&queue_mutex);
    if (q->front == -1) {
        /* Queue empty */
        // printf("Empty queue\n");
        pthread_mutex_unlock(&queue_mutex);
        return 1;
    }
    /* Queue not empty */
    pthread_mutex_unlock(&queue_mutex);
    return 0;
}

/*To check queue state*/
int queue_full(void) {
    pthread_mutex_lock(&queue_mutex);
    if ((q->rear + 1) % q->size == (q->rear)) {
        // printf("Full queue\n");
        pthread_mutex_unlock(&queue_mutex);
        return 1;
    }
    /* Queue not full */
    pthread_mutex_unlock(&queue_mutex);
    return 0;
}

/*To destroy the queue and free the resources*/
int queue_destroy(void) {
    free(q->elements);
    q->elements = NULL;
    free(q);
    pthread_mutex_destroy(&queue_mutex);
    pthread_cond_destroy(&queue_not_empty);
    pthread_cond_destroy(&queue_not_full);

    return 0;
}

void print_plane(struct plane *pln) {
    pthread_mutex_lock(&queue_mutex);
    printf("id_number = %d\n", pln->id_number);
    printf("time_action = %d\n", pln->time_action);
    printf("action = %d\n", pln->action);
    printf("last_flight = %d\n", pln->last_flight);
    pthread_mutex_unlock(&queue_mutex);
}

void display_queue(void) {
    pthread_mutex_lock(&queue_mutex);
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
    pthread_mutex_unlock(&queue_mutex);


}
