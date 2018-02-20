#include "person.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int filter(char id_ctrl, char* inputFile, char* outputFile);

int main(int argc, char *argv[]){
    /* Check that the arguments number is correct */
    if(argc != 4) {
        return 1;
    }

	return filter(argv[1][0], argv[2], argv[3]);
}

int filter(char id_ctrl, char* inputFile, char* outputFile) {
    /* Declaration of auxiliary person struct */
    Person input;

    /* We get the file descriptor of the input file */
    int inputFileDesc = open(inputFile, O_RDONLY);

    /* Check if an error ocurred when opening file */
    if(inputFileDesc < 0)
        return 1;

    /* We use stat system call to get the size of the binary file */
    struct stat st;
    stat(inputFile, &st);
    long binSize = st.st_size;

    /* We calculate the number of entries that the binary file has */
    long fileRows = binSize/sizeof(Person);

    /* Looks for the first location of the input file in memory */
    lseek(inputFileDesc, 0, SEEK_SET);

    int outputFileDesc = creat(outputFile, 0664);

    if(outputFileDesc < 0)
        return 1;

    lseek(outputFileDesc, 0, SEEK_SET);
    
    int i;

    /* This loop will repeat the number of structs contained in the binary
     * file */
    for(i = 0; i < fileRows; i++) {
        /* This line read each struct in the file and save the value in the
         * auxiliary struct input */
        read(inputFileDesc, &input, sizeof(Person));
        
        if(input.id_ctrl == id_ctrl) {
            write(outputFileDesc, &input, sizeof(Person));
        }
    }

    /* Close files */
    close(inputFileDesc);
    close(outputFileDesc);

    return 0;

}
