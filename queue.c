#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

typedef struct queue {
        int front, rear;

        int size;
        struct plane **elements;
} Queue;

Queue *q;

/*To create a queue*/
int queue_init(int size){
        q = (Queue*)malloc(sizeof(Queue));

        q->front = -1;
        q->rear = -1;

        q->size = size;
        q->elements = (struct plane**)malloc(size*sizeof(struct plane*));

        return 0;
}

/* To Enqueue an element*/
int queue_put(struct plane* x) {
        if ((q->front == 0 && q->rear == q->size-1) ||
            (q->rear == (q->front-1)%(q->size-1))) {
                printf("Full queue\n");
                return -1;
        } else if (q->front == -1) {
                q->front = q->rear = 0;
                q->elements[q->rear] = x;
                //memcpy(q->elements[q->rear], x, sizeof(struct plane));
        } else if (q->rear == q->size-1 && q->front != 0) {
                q->rear = 0;
                q->elements[q->rear] = x;
                //memcpy(q->elements[q->rear], x, sizeof(struct plane));
        } else {
                q->rear++;
                q->elements[q->rear] = x;
                //memcpy(q->elements[q->rear], x, sizeof(struct plane));
        }
        return 0;
}


/* To Dequeue an element.*/
struct plane* queue_get(void) {
        if (q->front == -1) {
                printf("Empty queue\n");
                return NULL;
        }

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

        return element;
}


/*To check queue state*/
int queue_empty(void){
        if (q->front == -1) {
                /* Queue empty */
                printf("Empty queue\n");
                return 1;
        }
        /* Queue not empty */
        return 0;
}

/*To check queue state*/
int queue_full(void){
        if ((q->front == 0 && q->rear == q->size-1) ||
            (q->rear == (q->front-1)%(q->size-1))) {
                printf("Full queue\n");
                return 1;
        }
        /* Queue not full */
        return 0;
}

/*To destroy the queue and free the resources*/
int queue_destroy(void){
        for (size_t i = 0; i < q->size; i++) {
                free(q->elements[i]);
                q->elements[i] = NULL;
        }
        free(q->elements);
        q->elements = NULL;
        free(q);
        q = NULL;

        return 0;
}
