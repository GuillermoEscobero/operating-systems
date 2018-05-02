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
    /* Check that the size of the queue is not 0 */
    if (size == 0) {
        fprintf(stderr, "%s\n\n", "ERROR: The size cannot be 0");
        return -1;
    }

    /* Allocate  memory for the Queue structure */
    q = (Queue *) malloc(sizeof(Queue));
    if (!q) {
        fprintf(stderr, "ERROR while initializing queue\n");
        return -1;
    }
    /* Initialize the values of the front and rear elements to -1 */
    q->front = -1;
    q->rear = -1;

    /* Set the size of the queue using the argument passed to the program */
    q->size = size;

    /* Allocate memory for the planes that will be stored in the queue */
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
    /* Lock the mutex to avoid other queue functions to access/edit it at the same time */
    pthread_mutex_lock(&queue_mutex);

    /* While the queue is full, wait until the not full signal is received */
    while (queue_full() == 1) {
        pthread_cond_wait(&queue_not_full, &queue_mutex);
    }
    printf("[QUEUE] Storing plane with id %d\n", x->id_number);

    /* Make up space for the new element */
    q->rear = (q->rear + 1) % q->size;
    /* Introduce the element into the queue */
    q->elements[q->rear] = x;

    /* If the front element has still has value -1, set the rear to the front to show that
     * only one element is stored in the circular queue */
    if (q->front == -1) {
        q->front = q->rear;
    }
    /* Send a signal to whichever thread is listening to show that the list is no longer empty */
    pthread_cond_signal(&queue_not_empty);

    /* Unlock the mutex to allow other queue functions to access/edit the queue */
    pthread_mutex_unlock(&queue_mutex);
    return 0;
}


/* To Dequeue an element.*/
struct plane *queue_get(void) {
    /* Lock the mutex to avoid other queue functions to access/edit it at the same time */
    pthread_mutex_lock(&queue_mutex);

    /* While the queue is empty, wait until the not empty signal is received */
    while (queue_empty() == 1) {
        printf("[CONTROL] Waiting for planes in empty queue\n");
        pthread_cond_wait(&queue_not_empty, &queue_mutex);
    }

    /* Get the element from the queue */
    struct plane *element = q->elements[q->front];

    /* If the element is the same at the beginning and at the end of the circular queue, the queue
     * is empty and a -1 is set in both sides to show it */
    if (q->front == q->rear) {
        q->front = -1;
        q->rear = -1;
    } else {
        /* If not, simply delete the space previously occupied by the element */
        q->front = (q->front + 1) % q->size;
    }
    printf("[QUEUE] Getting plane with id %d\n", element->id_number);

    /* Send a signal to whichever thread is listening to show that the list is no longer full */
    pthread_cond_signal(&queue_not_full);

    /* Unlock the mutex to allow other queue functions to access/edit the queue */
    pthread_mutex_unlock(&queue_mutex);
    return element;
}


/*To check queue state*/
int queue_empty(void) {
    /* If the front element has a -1, the queue is empty (a 1 is returned) */
    if (q->front == -1) {
        return 1;
    }
    return 0;
}

/*To check queue state*/
int queue_full(void) {
    /* If the next element from the end equals the one in the front, the queue is full (a 1 is returned) */
    if ((q->rear + 1) % q->size == (q->front)) {
        return 1;
    }
    return 0;
}

/*To destroy the queue and free the resources*/
int queue_destroy(void) {
    /* Free the elements array */
    free(q->elements);

    /* Delete the reference to the pointer */
    q->elements = NULL;

    /* Free the space allocated to the Queue structure */
    free(q);

    /* Delete both the conditions and the mutex used in queue.c */
    if (pthread_mutex_destroy(&queue_mutex) != 0) {
        fprintf(stderr, "%s\n", "ERROR when destroying the queue mutex");
        return -1;
    }
    if (pthread_cond_destroy(&queue_not_empty) != 0) {
        fprintf(stderr, "%s\n", "ERROR when destroying the queue_not_empty condition");
        return -1;
    }

    if (pthread_cond_destroy(&queue_not_full) != 0){
        fprintf(stderr, "%s\n", "ERROR when destroying the queue_not_empty condition");
        return -1;
    }

    return 0;
}
