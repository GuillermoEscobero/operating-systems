#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "queue.h"

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t queue_not_full = PTHREAD_COND_INITIALIZER;

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
    //FIXME: esto no deberia ir aqui, no?
    pthread_mutex_lock(&queue_mutex);
    while (queue_empty() == 1) {
        printf("[CONTROL] Waiting for planes in empty queue\n");
        pthread_cond_wait(&queue_not_empty, &queue_mutex);
    }
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
    if (q->front == -1) {
        /* Queue empty */
        return 1;
    }
    /* Queue not empty */
    return 0;
}

/*To check queue state*/
int queue_full(void) {
    if ((q->rear + 1) % q->size == (q->front)) {
        //FIXME: cuando el buffer es de 2 no comprueba esto
        return 1;
    }
    /* Queue not full */
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