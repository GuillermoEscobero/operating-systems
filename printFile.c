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

    /* We sum the size of each field separately to avoid padding symbols for
     * align purposes */
    long fileRows = binSize/(sizeof(input.name) + sizeof(input.age) +
            sizeof(input.id) + sizeof(input.id_ctrl) + sizeof(input.salary));

    /* Looks for the first location of the input file in memory */
    lseek(inputFileDesc, 0, SEEK_SET);

    int i;

    /* This loop will repeat the number of structs contained in the binary
     * file */
    for(i = 0; i < fileRows; i++) {
        /* These lines read each field of the current struct in the file
         * and save the value in the struct input */
        read(inputFileDesc, &input.name, sizeof(input.name));
        read(inputFileDesc, &input.age, sizeof(input.age));
        read(inputFileDesc, &input.id, sizeof(input.id));
        read(inputFileDesc, &input.id_ctrl, sizeof(input.id_ctrl));
        read(inputFileDesc, &input.salary, sizeof(input.salary));

        /* Prints in std output the current entry of the binary file */
        printf("%s\t%d\t%.8u\t%c\t%.0f\n", input.name, input.age, input.id,
                input.id_ctrl, input.salary);
    }

    /* Close input file */
    close(inputFileDesc);

    return 0;

}
