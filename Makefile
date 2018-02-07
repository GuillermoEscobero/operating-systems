all:
	gcc -Wall -ansi -o printFile printFile.c 
	gcc -Wall -ansi -o statistics statistics.c
	gcc -Wall -ansi -o split split.c 
	gcc -Wall -ansi -o filter filter.c
	gcc -Wall -ansi -o combine combine.c

clean:
	rm -f printFile statistics split filter combine
