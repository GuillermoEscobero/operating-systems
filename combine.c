#include "person.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int combine(char* inputFile1, char* inputFile2, char* outputFile);

int main(int argc, char *argv[]){
    /* Check that the arguments number is correct */
    if(argc != 4) {
        return 1;
    }

	return combine(argv[1], argv[2], argv[3]);
}

int combine(char* inputFile1, char* inputFile2, char* outputFile) {
    /* Declaration of auxiliary person structs */
    Person input1, input2;

    /* We get the file descriptor of the input files */
    int inputFileDesc1 = open(inputFile1, O_RDONLY);
    int inputFileDesc2 = open(inputFile2, O_RDONLY);
    
    /* Check if an error ocurred when opening file */
    if(inputFileDesc1 < 0 || inputFileDesc2 < 0)
        return 1;

    /* We use stat system call to get the size of the binary files */
    struct stat st;

    stat(inputFile1, &st);
    long binSize1 = st.st_size;

    stat(inputFile2, &st);
    long binSize2 = st.st_size;

    /* We calculate the number of entries that the binary file has */
    long fileRows1 = binSize1/sizeof(Person);
    long fileRows2 = binSize2/sizeof(Person);

    /* Looks for the end location of the input files in memory, and with an 
     * offset to put the descriptor in the beginning of the last struct */
    lseek(inputFileDesc1, -sizeof(Person), SEEK_END);
    lseek(inputFileDesc2, -sizeof(Person), SEEK_END);

    /* We create the output file, if exists, it is truncated */
    int outputFileDesc = creat(outputFile, 0664);

    /* Check if an error ocurred when creating file */
    if(outputFileDesc < 0)
        return 1;

    /* File descriptor to the beginning of output file */
    lseek(outputFileDesc, 0, SEEK_SET);
    
    int i, minRows, maxRows;
    int bigDesc;

    /* We compare the input files to get the descriptor of the bigger one */
    if(fileRows1 <= fileRows2) {
        minRows = fileRows1;
        maxRows = fileRows2;
        bigDesc = inputFileDesc2;
    } else {
        minRows = fileRows2;
        maxRows = fileRows1;
        bigDesc = inputFileDesc1;
    }

    /* This loop will repeat the number of structs contained in the small
     * binary file */
    for(i = 0; i < minRows; i++) {
        /* This line read each struct in the file and save the value in the
         * auxiliary struct input */
        read(inputFileDesc1, &input1, sizeof(Person));
        read(inputFileDesc2, &input2, sizeof(Person));

        /* The descriptor is moved two spaces of Person struct, because
         * read syscall has moved the descriptor forward one struct */
        lseek(inputFileDesc1, -2*sizeof(Person), SEEK_CUR);
        lseek(inputFileDesc2, -2*sizeof(Person), SEEK_CUR);

        /* The structs are written to the output file */
        write(outputFileDesc, &input1, sizeof(Person));
        write(outputFileDesc, &input2, sizeof(Person));
    }

    /* This loop will write the structs that lasts in the big binary file */
    for(i = 0; i < (maxRows-minRows); i++) {
        read(bigDesc, &input1, sizeof(Person));
        lseek(bigDesc, -2*sizeof(Person), SEEK_CUR);
        write(outputFileDesc, &input1, sizeof(Person));
    }

    /* Close files */
    close(inputFileDesc1);
    close(inputFileDesc2);
    close(outputFileDesc);

    return 0;

}
