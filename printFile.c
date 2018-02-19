#include "person.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int readBinary(char* inputFile);

int main(int argc, char *argv[]){
    /* Check that the arguments number is correct */
    if(argc != 2) {
        return 1;
    }

	return readBinary(argv[1]);
}

int readBinary(char* inputFile) {
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

    int i;

    /* This loop will repeat the number of structs contained in the binary
     * file */
    for(i = 0; i < fileRows; i++) {
        /* This line read each struct in the file and save the value in the
         * auxiliary struct input */
        read(inputFileDesc, &input, sizeof(Person));

        /* Prints in std output the current entry of the binary file */
        printf("%s\t%d\t%.8u\t%c\t%.0f\n", input.name, input.age, input.id,
                input.id_ctrl, input.salary);
    }

    /* Close input file */
    close(inputFileDesc);

    return 0;

}
