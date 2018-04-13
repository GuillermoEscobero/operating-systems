#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include <pthread.h>
#include <unistd.h>

#define NUM_TRACKS 1

pthread_mutex_t mut_id;

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

int main(int argc, char ** argv) {

    print_banner();
	
	print_end();

    return 0;
}
