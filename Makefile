CC=gcc
FLAGS=-g -Wall -Werror
OBJ= queue airport
LIBS= -pthread

all:  $(OBJ)
	@echo "***************************"
	@echo "Compilation successfully!"
	@echo ""

queue: queue.c
	$(CC) -c queue.c

airport:	arcport.c
	$(CC) $(CFLAGS)  $(LIBS)  -o arcport  arcport.c queue.c

#load:
#	ld -o process queue.o

clean:
	rm -f arcport *.o
	@echo "***************************"
	@echo "Deleted files!"
	@echo  ""

