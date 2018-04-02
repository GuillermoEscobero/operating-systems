 #
 # Makefile
 #
 # Minishell "make" source
 # Regenerate msh recompiling only modified sources
 #
 # DO NOT MODIFY THIS FILE
 #

CC	= gcc
CFLAGS	= -Wall -g
YFLAGS	= -d
LFLAGS	=

OBJS	= parser.o scanner.o y.o msh.o

all: msh

msh: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(CLION_EXE_DIR)$(OBJS)

clean:
	rm -f *.tab.? *.o *.bak *~ parser.c scanner.c core msh

cleanall: clean
	rm -f :* freefds* nofiles* killmyself* sigdfl*

depend:
	makedepend msh.c parser.y scanner.l
